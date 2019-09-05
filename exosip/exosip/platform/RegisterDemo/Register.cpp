#if defined(__arc__)
#define LOG_PERROR 1
#include <includes_api.h>
#include <os_cfg_pub.h>
#endif

#if !defined(_WIN32) && !defined(_WIN32_WCE)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <syslog.h>
#ifdef OSIP_MT
#include <pthread.h>
#endif
#endif

#ifdef _WIN32_WCE
#include <winsock2.h>
#endif

#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>

#if !defined(_WIN32) && !defined(_WIN32_WCE) && !defined(__arc__)
#define _GNU_SOURCE
#include <getopt.h>
#endif

#define PROG_NAME "sipreg"
#define PROG_VER  "1.0"
#define UA_STRING "SipReg v" PROG_VER
#define SYSLOG_FACILITY LOG_DAEMON

#pragma comment(lib, "eXosip2.lib")
#pragma comment(lib, "libcares.lib")
#pragma comment(lib, "osip2.lib")
#pragma comment(lib, "osipparser2.lib")

#ifdef ANDROID
#include <android/log.h>
#define LOGI(format,...)  //__android_log_print(ANDROID_LOG_INFO ,"hello_hl","file[%s] line[%d] "format"",__FILE__, __LINE__ ,##__VA_ARGS__)
#define LOGE(format,...)  __android_log_print(ANDROID_LOG_ERROR,"hello_hl","file[%s] line[%d] "format"",__FILE__, __LINE__ ,##__VA_ARGS__)
#define LOG(format, ...) fprintf(stdout, format, ##__VA_ARGS__)  
#define LOG(format, args...) fprintf(stdout, format, ##args)  
#else
#include <stdio.h>
//#define LOGI(format,...)  printf("file[%s] line[%d] " format "\n",__FILE__, __LINE__ , ##__VA_ARGS__)
//#define LOGE(format,...)  printf("file[%s] line[%d] " format "\n",__FILE__, __LINE__ , ##__VA_ARGS__)
//#define LOGE(format,...)  printf("file[%s] line[%d] " format "\n",__FILE__, __LINE__ , ##__VA_ARGS__)
#define LOGE(format, ...)  printf("file[%s] line[%d] " format "\n",__FILE__, __LINE__ , ##__VA_ARGS__)
#endif


#if defined(_WIN32) || defined(_WIN32_WCE)
static void syslog_wrapper(int a, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
}
#define LOG_INFO 0
#define LOG_ERR 0
#define LOG_WARNING 0
#define LOG_DEBUG 0

#elif defined(LOG_PERROR)
#define syslog_wrapper    syslog
#else
#define syslog_wrapper(a,b...) fprintf(stderr,b);fprintf(stderr,"\n")
#endif


static void usage(void);
#ifdef OSIP_MT
static void *register_proc(void *arg);
#endif

static void
usage(void)
{
	printf("Usage: " PROG_NAME " [required_options] [optional_options]\n"
		"\n\t[required_options]\n"
		"\t-r --proxy\tsip:proxyhost[:port]\n"
		"\t-u --from\tsip:user@host[:port]\n"
		"\n\t[optional_options]\n"
		"\t-c --contact\tsip:user@host[:port]\n"
		"\t-d --debug (log to stderr and do not fork)\n"
		"\t-e --expiry\tnumber (default 3600)\n"
		"\t-f --firewallip\tN.N.N.N\n"
		"\t-h --help\n"
		"\t-l --localip\tN.N.N.N (force local IP address)\n"
		"\t-p --port\tnumber (default 5060)\n"
		"\t-U --username\tauthentication username\n"
		"\t-P --password\tauthentication password\n");
}

typedef struct regparam_t
{
	int regid;
	int expiry;
	int auth;
} regparam_t;

#ifdef OSIP_MT
static void *
register_proc(void *arg)
{
	struct regparam_t *regparam = arg;
	int reg;

	for (;;)
	{
#ifdef _WIN32_WCE
		Sleep((regparam->expiry / 2) * 1000);
#else
		sleep(regparam->expiry / 2);
#endif
		eXosip_lock();
		reg = eXosip_register_send_register(regparam->regid, NULL);
		if (0 > reg)
		{
#ifdef _WIN32_WCE
			fprintf(stdout, "eXosip_register: error while registring");
#else
			perror("eXosip_register");
#endif
			exit(1);
		}
		regparam->auth = 0;
		eXosip_unlock();
	}
	return NULL;
}
#endif

#ifdef _WIN32_WCE
//int WINAPI WinMain(HINSTANCE hInstance,
//	HINSTANCE hPrevInstance,
//	LPTSTR    lpCmdLine,
//	int       nCmdShow)

//int _stdcall WinMain(
//	HINSTANCE hInstance,
//	HINSTANCE hPrevInstance,
//	LPSTR lpCmdLine,
//	int nCmdShow
//)
int main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int c;
	int port = 5060;
	char *contact = NULL;
	char *fromuser = NULL;
	const char *localip = NULL;
	const char *firewallip = NULL;
	char *proxy = NULL;
#if !defined(__arc__)
	struct servent *service;
#endif
	char *username = NULL;
	char *password = NULL;
	struct regparam_t regparam = { 0, 3600, 0 };
#ifdef OSIP_MT
	struct osip_thread *register_thread;
#endif
	int debug = 0;
	int nofork = 0;

#ifdef _WIN32_WCE
	//proxy = osip_strdup("sip:sip.antisip.com");
	proxy = osip_strdup("sip:39.106.13.209:5060");
	fromuser = osip_strdup("sip:803@39.106.13.209:5060");
#else
	for (;;)
	{
#define short_options "c:de:f:hl:p:r:u:U:P:"
#ifdef _GNU_SOURCE
		int option_index = 0;
		static struct option long_options[] = {
			{ "contact", required_argument, NULL, 'c' },
			{ "debug", no_argument, NULL, 'd' },
			{ "expiry", required_argument, NULL, 'e' },
			{ "firewallip", required_argument, NULL, 'f' },
			{ "from", required_argument, NULL, 'u' },
			{ "help", no_argument, NULL, 'h' },
			{ "localip", required_argument, NULL, 'l' },
			{ "port", required_argument, NULL, 'p' },
			{ "proxy", required_argument, NULL, 'r' },
			{ "username", required_argument, NULL, 'U' },
			{ "password", required_argument, NULL, 'P' },
			{ NULL, 0, NULL, 0 }
		};

		c = getopt_long(argc, argv, short_options, long_options, &option_index);
#else
		c = getopt(argc, argv, short_options);
#endif
		if (c == -1)
			break;

		switch (c)
		{
		case 'c':
			contact = optarg;
			break;
		case 'd':
			nofork = 1;
#ifdef LOG_PERROR
			debug = LOG_PERROR;
#endif
			break;
		case 'e':
			regparam.expiry = atoi(optarg);
			break;
		case 'f':
			firewallip = optarg;
			break;
		case 'h':
			usage();
			exit(0);
		case 'l':
			localip = optarg;
			break;
		case 'p':
#if !defined(__arc__)
			service = getservbyname(optarg, "udp");
			if (service)
				port = ntohs(service->s_port);
			else
				port = atoi(optarg);
#else
			port = atoi(optarg);
#endif
			break;
		case 'r':
			proxy = optarg;
			break;
		case 'u':
			fromuser = optarg;
			break;
		case 'U':
			username = optarg;
			break;
		case 'P':
			password = optarg;
			break;
		default:
			break;
		}
	}
#endif

	LOGE("111111");
	if (!proxy || !fromuser)
	{
		usage();
		exit(1);
	}

#ifndef _WIN32_WCE
	if (!nofork)
	{
		int cpid = fork();

		if (cpid)
			exit(0);

		LOGE("22222");
		close(0);
		close(1);
		close(2);

	}
#endif

#if 0
	openlog(PROG_NAME, LOG_PID | debug, SYSLOG_FACILITY);
#endif

	LOGE(" up and running");
	LOGE("proxy: %s", proxy);
	LOGE("fromuser: %s", fromuser);
	LOGE("contact: %s", contact);
	LOGE("expiry: %d", regparam.expiry);
	LOGE("local port: %d", port);
	LOGE("passwd :%s", password);

	if (debug>0)
		TRACE_INITIALIZE(6, NULL);
	eXosip_t* eCtx = eXosip_malloc();
	if (eXosip_init(eCtx))
	{
		syslog_wrapper(LOG_ERR, "eXosip_init failed");
		exit(1);
	}
	if (eXosip_listen_addr(eCtx, IPPROTO_UDP, NULL, port, AF_INET, 0))
	{
		syslog_wrapper(LOG_ERR, "eXosip_listen_addr failed");
		exit(1);
	}

	if (localip)
	{
		syslog_wrapper(LOG_INFO, "local address: %s", localip);
		eXosip_masquerade_contact(eCtx, localip, port);
	}

	if (firewallip)
	{
		syslog_wrapper(LOG_INFO, "firewall address: %s:%i", firewallip, port);
		eXosip_masquerade_contact(eCtx, firewallip, port);
	}

	eXosip_set_user_agent(eCtx, UA_STRING);

	if (username && password)
	{
		syslog_wrapper(LOG_INFO, "username: %s", username);
		syslog_wrapper(LOG_INFO, "password: [removed]");
		if (eXosip_add_authentication_info(eCtx, username, username, password, NULL, NULL))
		{
			syslog_wrapper(LOG_ERR, "eXosip_add_authentication_info failed");
			exit(1);
		}
	}

	{
		osip_message_t *reg = NULL;
		int i;

		regparam.regid = eXosip_register_build_initial_register(eCtx, fromuser, proxy, contact, regparam.expiry * 2, &reg);
		LOGE("33333 regid[%d] ", regparam.regid);
		if (regparam.regid < 1) {
			syslog_wrapper(LOG_ERR,	"eXosip_register_build_initial_register failed");
			LOGE("66666");
			exit(1);
		}
		i = eXosip_register_send_register(eCtx, regparam.regid, reg);
		if (i != 0) {
			syslog_wrapper(LOG_ERR, "eXosip_register_send_register failed");
			LOGE("55555");
			exit(1);
		}
		LOGE("44444");
	}

#ifdef OSIP_MT
	register_thread = osip_thread_create(20000, register_proc, &regparam);
	if (register_thread == NULL)
	{
		syslog_wrapper(LOG_ERR, "pthread_create failed");
		exit(1);
	}
#endif

	for (;;)
	{
		eXosip_event_t *event;


		if (!(event = eXosip_event_wait(eCtx, 0, 50)))
		{
#ifndef OSIP_MT
			LOGE("77777");
			eXosip_execute(eCtx);
			eXosip_automatic_action(eCtx);
#endif
			osip_usleep(10000);
			continue;
		}

#ifndef OSIP_MT
		eXosip_execute(eCtx);
#endif

		LOGE("88888");

		eXosip_automatic_action(eCtx);
		switch (event->type)
		{
			int res;
		case EXOSIP_MESSAGE_NEW:
			if(MSG_IS_REGISTER(event->request))
				LOGE("received new registration");
			else if(MSG_IS_MESSAGE(event->request))
				LOGE("received new message");
			break;
		case EXOSIP_REGISTRATION_SUCCESS:
			LOGE("registrered successfully");
			break;
		case EXOSIP_REGISTRATION_FAILURE:
			regparam.auth = 1;
			//注册收到401之后，再次提交授权信息
			//res = eXosip_add_authentication_info(username,username,password, NULL, NULL);
			LOGE("401!!!!");
			break;
		//case EXOSIP_REGISTRATION_TERMINATED:
		case EXOSIP_CALL_CLOSED:
			LOGE("Registration terminated\n");
			break;
		default:
			LOGE("recieved unknown eXosip event (type, did, cid) = (%d, %d, %d)", event->type, event->did, event->cid);
		}
		eXosip_event_free(event);
	}
}