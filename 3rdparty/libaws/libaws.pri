DEPENDPATH += $$PWD/. \
              $$PWD/src \
              $$PWD/include/libaws \
              $$PWD/src/api \
              $$PWD/src/logging \
              $$PWD/src/s3 \
#             $$PWD/src/sdb \
#             $$PWD/src/sqs

INCLUDEPATH +=  $$PWD/.  $$PWD/include/ $$PWD/include/libaws $$PWD/src
    #$$PWD/src/sdb $$PWD/src/sqs

# Input
HEADERS += $$PWD/src/awsconnection.h \
           $$PWD/src/awsquerycallback.h \
           $$PWD/src/awsqueryconnection.h \
           $$PWD/src/awsqueryresponse.h \
           $$PWD/src/callingformat.h \
           $$PWD/src/canonizer.h \
           $$PWD/src/common.h \
           $$PWD/src/curlstreambuf.h \
           $$PWD/src/requestheadermap.h \
           $$PWD/src/response.h \
           $$PWD/src/util.h \
           $$PWD/include/libaws/aws.h \
           $$PWD/include/libaws/awsconnectionfactory.h \
           $$PWD/include/libaws/awstime.h \
           $$PWD/include/libaws/awsversion.h \
           $$PWD/include/libaws/common.h \
           $$PWD/include/libaws/connectionpool.h \
           $$PWD/include/libaws/exception.h \
           $$PWD/include/libaws/mutex.h \
           $$PWD/include/libaws/s3connection.h \
           $$PWD/include/libaws/s3exception.h \
           $$PWD/include/libaws/s3response.h \
           $$PWD/include/libaws/sdbconnection.h \
           $$PWD/include/libaws/sdbexception.h \
           $$PWD/include/libaws/sdbresponse.h \
           $$PWD/include/libaws/smart_ptr.h \
           $$PWD/include/libaws/sqsconnection.h \
           $$PWD/include/libaws/sqsexception.h \
           $$PWD/include/libaws/sqsresponse.h \
           $$PWD/src/api/awsconnectionfactoryimpl.h \
           $$PWD/src/api/s3connectionimpl.h \
           $$PWD/src/api/sdbconnectionimpl.h \
           $$PWD/src/api/sqsconnectionimpl.h \
           $$PWD/src/logging/logger.hh \
           $$PWD/src/logging/loggerconfig.hh \
           $$PWD/src/logging/loggermanager.hh \
           $$PWD/src/logging/logging.hh \
           $$PWD/src/s3/s3callbackwrapper.h \
           $$PWD/src/s3/s3connection.h \
           $$PWD/src/s3/s3handler.h \
           $$PWD/src/s3/s3object.h \
           $$PWD/src/s3/s3response.h \
           $$PWD/src/sdb/sdbconnection.h \
           $$PWD/src/sdb/sdbhandler.h \
           $$PWD/src/sdb/sdbresponse.h \
           $$PWD/src/sqs/sqsconnection.h \
           $$PWD/src/sqs/sqshandler.h \
           $$PWD/src/sqs/sqsresponse.h

SOURCES += $$PWD/src/awsversion.cpp \
           $$PWD/src/awsconnection.cpp \
           $$PWD/src/awsquerycallback.cpp \
           $$PWD/src/awsqueryconnection.cpp \
           $$PWD/src/awstime.cpp \
           $$PWD/src/callingformat.cpp \
           $$PWD/src/canonizer.cpp \
           $$PWD/src/curlstreambuf.cpp \
           $$PWD/src/exception.cpp \
           $$PWD/src/requestheadermap.cpp \
           $$PWD/src/response.cpp \
           $$PWD/src/api/awsconnectionfactory.cpp \
           $$PWD/src/api/awsconnectionfactoryimpl.cpp \
           $$PWD/src/api/connectionpool.cpp \
           $$PWD/src/api/mutex.cpp \
           $$PWD/src/api/s3connectionimpl.cpp \
           $$PWD/src/api/s3response.cpp \
           $$PWD/src/api/sdbconnectionimpl.cpp \
           $$PWD/src/api/sdbresponse.cpp \
           $$PWD/src/api/sqsconnectionimpl.cpp \
           $$PWD/src/api/sqsresponse.cpp \
           $$PWD/src/logging/logger.cc \
           $$PWD/src/logging/loggerconfig.cc \
           $$PWD/src/logging/loggermanager.cc \
           $$PWD/src/s3/s3s3connection.cpp \
           $$PWD/src/s3/s3s3exception.cpp \
           $$PWD/src/s3/s3s3handler.cpp \
           $$PWD/src/s3/s3s3object.cpp \
           $$PWD/src/s3/s3s3response.cpp \
           $$PWD/src/sdb/sdbsdbconnection.cpp \
           $$PWD/src/sdb/sdbsdbexception.cpp \
           $$PWD/src/sdb/sdbsdbhandler.cpp \
           $$PWD/src/sdb/sdbsdbresponse.cpp \
           $$PWD/src/sqs/sqssqsconnection.cpp \
           $$PWD/src/sqs/sqssqsexception.cpp \
           $$PWD/src/sqs/sqssqshandler.cpp \
           $$PWD/src/sqs/sqssqsresponse.cpp
