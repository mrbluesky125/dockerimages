# Only headers here, no library is wanted.
TEMPLATE = subdirs
VERSION = $$MODULE_VERSION
MODULE_INCNAME = QtPlatformHeaders

include(qwebglfunctions/qwebglfunctions.pri)

load(qt_module_headers)
#load(qt_docs)
load(qt_installs)
