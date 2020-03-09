/*
 * PyUtils.h
 *
 *  Created on: May 30, 2018
 *      Author: hongxu
 */

#ifndef SRC_PYEXT_PYUTILS_H_
#define SRC_PYEXT_PYUTILS_H_

#include <string>
#include <vector>
#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/stl_iterator.hpp>
namespace bp = boost::python;
using namespace std;


template<typename T>
inline vector <T> list_to_vector(const boost::python::object &iterable)
{
    return vector<T>(bp::stl_input_iterator<T>(iterable), bp::stl_input_iterator<T>());
}

template<class T>
inline bp::list vector_to_list(const vector <T> &vec)
{
    bp::list l;
    for (auto &i : vec)
    {
        l.append(i);
    }
    return l;
}

class ReleaseGIL
{
public:
    inline ReleaseGIL()
    {
        save_state = PyEval_SaveThread();
    }

    inline ~ReleaseGIL()
    {
        PyEval_RestoreThread(save_state);
    }
private:
    PyThreadState *save_state;
};

class AcquireGIL
{
public:
    inline AcquireGIL()
    {
        state = PyGILState_Ensure();
    }

    inline ~AcquireGIL()
    {
        PyGILState_Release(state);
    }
private:
    PyGILState_STATE state;
};

#define PYTHON_ERROR(TYPE, REASON) \
{ \
    PyErr_SetString(TYPE, REASON); \
    throw bp::error_already_set(); \
}

template<class T>
inline PyObject * managingPyObject(T *p)
{
    return typename bp::manage_new_object::apply<T *>::type()(p);
}

template<class Copyable>
bp::object generic__copy__(bp::object copyable)
{
    Copyable *newCopyable(new Copyable(bp::extract<const Copyable&>(copyable)));
    bp::object result(bp::detail::new_reference(managingPyObject(newCopyable)));
    bp::extract<bp::dict>(result.attr("__dict__"))().update(copyable.attr("__dict__"));
    return result;
}

template<class Copyable>
bp::object generic__deepcopy__(bp::object copyable, bp::dict memo)
{
    bp::object copyMod = bp::import("copy");
    bp::object deepcopy = copyMod.attr("deepcopy");

    Copyable *newCopyable(new Copyable(bp::extract<const Copyable&>(copyable)));
    bp::object result(bp::detail::new_reference(managingPyObject(newCopyable)));

    // HACK: copyableId shall be the same as the result of id(copyable) in Python -
    // please tell me that there is a better way! (and which ;-p)
    long copyableId = (long)(copyable.ptr());
    memo[copyableId] = result;

    bp::extract<bp::dict>(result.attr("__dict__"))().update(deepcopy(bp::extract<bp::dict>(copyable.attr("__dict__"))(), memo));
    return result;
}

#endif /* SRC_PYEXT_PYUTILS_H_ */
