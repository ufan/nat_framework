#[===============================================================================[.rst
# ./external/openssl/CMakeLists.txt -- NatFramework
#
# Find OpenSSL library.
# ^^^^^^^^^^^^^^^^^^^^
#
# Imported targets in global scope:
#
#   OpenSSL::SSL
#   OpenSSL::Crypto
#
#]===============================================================================]

# variables controlling the behavior of openssl
set(OPENSSL_USE_STATIC_LIBS      TRUE)

# set(OPENSSL_ROOT_DIR "/home/yong/openssl/1_72_0") # can be omitted and provided by user
find_package(OpenSSL REQUIRED)

# Make found targets globally available, in case of list file in subdirectory
if (OPENSSL_FOUND)
  set_target_properties(
    OpenSSL::SSL
    OpenSSL::Crypto
    PROPERTIES IMPORTED_GLOBAL TRUE)
endif ()
