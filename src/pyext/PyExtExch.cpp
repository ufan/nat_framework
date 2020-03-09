/*
 * PyExtExch.cpp
 *
 *  Created on: 2018年10月29日
 *      Author: hongxu
 */

#include <fstream>
#include "Logger.h"
#include "PyExtExch.h"
#include "PyUtils.h"

static int  s_pyHoldCnt = 0;
static bool s_isInitPy = false;
static void initPython(string interpreter, string python_path)
{
	if(not s_isInitPy && not Py_IsInitialized())
	{
		Py_SetProgramName((char*)interpreter.c_str());
		if(python_path.size())
		{
			Py_SetPythonHome((char*)python_path.c_str());
		}
		Py_Initialize();
		s_isInitPy = true;
	}
	s_pyHoldCnt ++;
}

static void releasePython()
{
	if(--s_pyHoldCnt <= 0)
	{
		if(s_isInitPy)
		{
			Py_Finalize();
			s_isInitPy = false;
		}
	}
}

CPyExtTdExch::CPyExtTdExch()
{

}

CPyExtTdExch::~CPyExtTdExch()
{
	releasePython();
}

bool CPyExtTdExch::init(const json& j_conf)
{
	string interpreter = j_conf["interpreter"];
	string python_file = j_conf["file"];
	string args = j_conf["args"].dump();
	string python_path;
	if(j_conf.find("PYTHONPATH") != j_conf.end())
	{
		python_path = j_conf["PYTHONPATH"];
	}

	ifstream in(python_file);
	if(!in)
	{
		LOG_ERR("read config file %s err.", python_file.c_str());
		return false;
	}
	string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();

	try
	{
		initPython(interpreter, python_path);
		bp::object main_module = bp::import("__main__");
		bp::dict main_namespace = bp::extract<bp::dict>(main_module.attr("__dict__"));
		bp::exec(content.c_str(), main_namespace, main_namespace);

		td_send_order_ = main_namespace["td_send_order"];
		td_del_order_ = main_namespace["td_del_order"];
		td_qry_base_info_ = main_namespace["td_qry_base_info"];
		td_qry_order_track_ = main_namespace["td_qry_order_track"];
		td_get_rtn_ = main_namespace["td_get_rtn"];
		td_on_tick_ = main_namespace["td_on_tick"];
		td_on_time_ = main_namespace["td_on_time"];
		td_switch_day_ = main_namespace["td_on_switch_day"];
		td_release_ = main_namespace["td_release"];

		bp::object td_init = main_namespace["td_init"];
		bool bret = bp::extract<bool>(td_init(args));
		if(not bret)
		{
			LOG_ERR("td_init failed.");
			return false;
		}
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
        return false;
    }

	return true;
}

void CPyExtTdExch::sendOrder(tOrderTrack ot)
{
	try
	{
		td_send_order_(ot);
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
}

void CPyExtTdExch::delOrder(int track_id)
{
	try
	{
		td_del_order_(track_id);
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
}

bool CPyExtTdExch::qryTradeBaseInfo()
{
	try
	{
		return td_qry_base_info_();
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
	return false;
}

bool CPyExtTdExch::qryOrderTrack(vector<tOrderTrack> &ot)
{
	try
	{
		bp::list l = bp::extract<bp::list>(td_qry_order_track_());
		for (int i = 0; i < bp::len(l); ++i)
		{
			ot.push_back(bp::extract<tOrderTrack>(l[i]));
		}
		return true;
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
	return false;
}

const tRtnMsg* CPyExtTdExch::getRtn()
{
	try
	{
		bp::object rtn = td_get_rtn_();
		if(rtn)
		{
			hold_rtn_ = *(bp::extract<const tRtnMsg*>(rtn));
			return &hold_rtn_;
		}
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
	return nullptr;
}


void CPyExtTdExch::on_tick(const UnitedMarketData* pumd)
{
	try
	{
		td_on_tick_(*pumd);
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
}

void CPyExtTdExch::on_time(long nano)
{
	try
	{
		td_on_time_(nano);
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
}

void CPyExtTdExch::on_switch_day(string day)
{
	try
	{
		td_switch_day_(day);
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
}

void CPyExtTdExch::release()
{
	try
	{
		td_release_();
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
}

// ----------- CPyExtMdExch -------------

CPyExtMdExch::CPyExtMdExch()
{

}

CPyExtMdExch::~CPyExtMdExch()
{
	releasePython();
}

bool CPyExtMdExch::init(const json& j_conf)
{
	string interpreter = j_conf["interpreter"];
	string python_file = j_conf["file"];
	string args = j_conf["args"].dump();
	string python_path;
	if(j_conf.find("PYTHONPATH") != j_conf.end())
	{
		python_path = j_conf["PYTHONPATH"];
	}

	ifstream in(python_file);
	if(!in)
	{
		LOG_ERR("read config file %s err.", python_file.c_str());
		return false;
	}
	string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();

	try
	{
		initPython(interpreter, python_path);
		bp::object main_module = bp::import("__main__");
		bp::dict main_namespace = bp::extract<bp::dict>(main_module.attr("__dict__"));
		bp::exec(content.c_str(), main_namespace, main_namespace);

		md_read_ = main_namespace["md_read"];
		md_get_subs_ = main_namespace["md_get_subs"];
		md_subs_ = main_namespace["md_subs"];
		md_unsubs_ = main_namespace["md_unsubs"];
		md_set_read_pos_ = main_namespace["md_set_read_pos"];
		md_release_ = main_namespace["md_release"];

		bp::object md_init = main_namespace["md_init"];
		bool bret = bp::extract<bool>(md_init(args));
		if(not bret)
		{
			LOG_ERR("md_init failed.");
			return false;
		}
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
        return false;
    }

	return true;
}

const UnitedMarketData* CPyExtMdExch::readMd(long &md_nano)
{
	try
	{
		bp::tuple res = bp::extract<bp::tuple>(md_read_());
		if(res[0])
		{
			md_nano = bp::extract<long>(res[1]);
			hold_tick_ = *(bp::extract<const UnitedMarketData*>(res[0]));
			return &hold_tick_;
		}
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
	return nullptr;
}

vector<string> CPyExtMdExch::getSubsInstr()
{
	try
	{
		bp::list l = bp::extract<bp::list>(md_get_subs_());
		return list_to_vector<string>(l);
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
    return {};
}

bool CPyExtMdExch::subscribe(const vector<string>& instr)
{
	try
	{
		return bp::extract<bool>(md_subs_(vector_to_list(instr)));
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
	return false;
}

bool CPyExtMdExch::unsubscribe(const vector<string>& instr)
{
	try
	{
		return bp::extract<bool>(md_unsubs_(vector_to_list(instr)));
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
	return false;
}

void CPyExtMdExch::setReadPos(long nano)
{
	try
	{
		md_set_read_pos_(nano);
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
}

void CPyExtMdExch::release()
{
	try
	{
		md_release_();
	}
	catch (bp::error_already_set& e)
	{
        PyErr_PrintEx(0);
    }
}

