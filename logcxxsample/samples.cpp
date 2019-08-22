#include <iostream>
#include <log4cxx/logstring.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <locale.h>
#include <log4cxx/basicconfigurator.h>

#pragma comment(lib, "log4cxx.lib")

using namespace log4cxx;
using namespace log4cxx::helpers;

int main() 
{
	//LoggerPtr logger(Logger::getLogger("LogTest1"));

	//BasicConfigurator::configure();
	//LOG4CXX_INFO(logger, "Hello ");
	//LOG4CXX_DEBUG(logger, "Log4Cxx");
	//printf("aaaaaaaaaaa");

	//加载log4cxx的配置文件，这里使用了属性文件
	PropertyConfigurator::configure("log4cxx.properties");

	//获得一个Logger，这里使用了RootLogger
	LoggerPtr rootLogger = Logger::getRootLogger();

	//发出INFO级别的输出请求
	//LOG4CXX_INFO(rootLogger, _T("日志系统启动了。。。。"));
	rootLogger->info("日志系统启动了。。。。"); //与上面那句话功能一样

	return 0;
}