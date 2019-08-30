//网上找的例子，目前正在研究pjsip

//PJLIB-UTIL初始化 ---> 创建 pool factory ---> 创建SIP endpoint ---> 创建SIP transport ---> 初始化transaction layer
//---> 初始化UA layer ---> 初始化 100rel module(处理临时响应) ---> 创建invite session module ---> 创建media endpoint ---> 创建media transport
#include <pjsip.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <pjsua_internal.h>

//static pjsip_module mod_pjsip =
//{
//	NULL, NULL,                     /* prev, next.              */
//	{ "mod-pjsip", 9 },         /* Name.                    */
//	-1,                             /* Id                       */
//	PJSIP_MOD_PRIORITY_APPLICATION, /* Priority                 */
//	NULL,                           /* load()                   */
//	NULL,                           /* start()                  */
//	NULL,                           /* stop()                   */
//	NULL,                           /* unload()                 */
//	&on_rx_request,                 /* on_rx_request()          */
//	NULL,                           /* on_rx_response()         */
//	NULL,                           /* on_tx_request.           */
//	NULL,                           /* on_tx_response()         */
//	NULL,                           /* on_tsx_state()           */
//};

int main(int argc, char* argv[]) {
	/* Then init PJLIB-UTIL: */
	pj_status_t status = pjlib_util_init();
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
	pjsua_data sip_config;
	int i = 0;
	/* Must create a pool factory before we can allocate any memory. */
	pj_caching_pool_init(&sip_config.cp, &pj_pool_factory_default_policy, 0);
	sip_config.pool = pj_pool_create(&sip_config.cp.factory, "pjsip-app", 1000, 1000, NULL);

	/* Create global endpoint: */
	{
		const pj_str_t *hostname;
		const char *endpt_name;

		/* Endpoint MUST be assigned a globally unique name.*/
		hostname = pj_gethostname();
		endpt_name = hostname->ptr;

		/* Create the endpoint: */

		status = pjsip_endpt_create(&sip_config.cp.factory, endpt_name,	&sip_config.endpt);
		PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
	}

	for (i = 0; i<sip_config.thread_count; ++i) 
	{
		pj_thread_create(sip_config.pool, "app", &sip_worker_thread, NULL,
			0, 0, &sip_config.sip_thread[i]);
	}
	/*
	*  Add UDP transport, with hard-coded port
	*  Alternatively, application can use pjsip_udp_transport_attach() to
	*  start UDP transport, if it already has an UDP socket (e.g. after it
	*  resolves the address with STUN).
	*  */
	{
		/* ip address of localhost */
		pj_sockaddr addr;

		pj_sockaddr_init(AF, &addr, NULL, (pj_uint16_t)SIP_PORT);

		if (AF == pj_AF_INET()) {
			status = pjsip_udp_transport_start(sip_config.g_endpt, &addr.ipv4, NULL,
				1, &sip_config.tp);
		}
		else if (AF == pj_AF_INET6()) {
			status = pjsip_udp_transport_start6(sip_config.g_endpt, &addr.ipv6, NULL,
				1, &sip_config.tp);
		}
		else {
			status = PJ_EAFNOTSUP;
		}

		if (status != PJ_SUCCESS) {
			app_perror(THIS_FILE, "Unable to start UDP transport", status);
			return 1;
		}
		PJ_LOG(3, (THIS_FILE, "SIP UDP listening on %.*s:%d",
			(int)sip_config.tp->local_name.host.slen, sip_config.tp->local_name.host.ptr,
			sip_config.tp->local_name.port));
	}
	/* Set transport state callback */
	{
		pjsip_tp_state_callback tpcb;
		pjsip_tpmgr *tpmgr;

		tpmgr = pjsip_endpt_get_tpmgr(sip_config.g_endpt);
		tpcb = pjsip_tpmgr_get_state_cb(tpmgr);

		if (tpcb != &on_tp_state_callback) {
			sip_config.old_tp_cb = tpcb;
			pjsip_tpmgr_set_state_cb(tpmgr, &on_tp_state_callback);
		}
	}
	/*
	*  Init transaction layer.
	*  This will create/initialize transaction hash tables etc.
	*  */
	status = pjsip_tsx_layer_init_module(sip_config.g_endpt);
	if (status != PJ_SUCCESS) {
		app_perror(THIS_FILE, "Unable to initialize transaction layer", status);
		return status;
	}
	/*
	*  Initialize UA layer module.
	*  This will create/initialize dialog hash tables etc.
	*  */
	status = pjsip_ua_init_module(sip_config.g_endpt, NULL);
	if (status != PJ_SUCCESS) {
		app_perror(THIS_FILE, "Unable to initialize UA layer", status);
		return status;
	}

	status = pjsip_100rel_init_module(sip_config.g_endpt);
	if (status != PJ_SUCCESS) {
		app_perror(THIS_FILE, "Unable to initialize 100rel", status);
		return status;
	}
	/*
	*  Init invite session module.
	*  The invite session module initialization takes additional argument,
	*  i.e. a structure containing callbacks to be called on specific
	*  occurence of events.
	*  We use on_media_update() callback in this application to start
	*  media transmission.
	*  */
	{
		/* Init the callback for INVITE session: */
		pj_bzero(&sip_config.inv_cb, sizeof(sip_config.inv_cb));
		sip_config.inv_cb.on_state_changed = &call_on_state_changed;
		sip_config.inv_cb.on_new_session = &call_on_forked;
		sip_config.inv_cb.on_media_update = &call_on_media_update;

		/* Initialize invite session module:  */
		status = pjsip_inv_usage_init(sip_config.g_endpt, &sip_config.inv_cb);
		if (status != PJ_SUCCESS) {
			app_perror(THIS_FILE, "Unable to initialize invite session module", status);
			return 1;
		}
	}
	/*  Register our module to receive incoming requests. */
	status = pjsip_endpt_register_module(sip_config.g_endpt, &mod_pjsip);
	if (status != PJ_SUCCESS) {
		app_perror(THIS_FILE, "Unable to register pjsip app module", status);
		return 1;
	}

	/*
	*  Initialize media endpoint.
	*  This will implicitly initialize PJMEDIA too.
	*  */
	status = pjmedia_endpt_create(&sip_config.cp.factory, NULL, 1, &sip_config.g_med_endpt);
	if (status != PJ_SUCCESS) {
		app_perror(THIS_FILE, "Unable to create media endpoint", status);
		return 1;
	}

	/* Add PCMA/PCMU codec to the media endpoint. */
	status = pjmedia_codec_g711_init(sip_config.g_med_endpt);
	if (status != PJ_SUCCESS) {
		app_perror(THIS_FILE, "Unable to add codec", status);
		return 1;
	}

	/*  Create media transport used to send/receive RTP/RTCP socket.
	*  One media transport is needed for each call. Application may
	*  opt to re-use the same media transport for subsequent calls.
	*  */
	rtp_port = (pj_uint16_t)(sip_config.rtp_start_port & 0xFFFE);
	/* Init media transport for all calls. */
	for (i = 0, count = 0; i<sip_config.max_calls; ++i, ++count) {
		unsigned j;
		for (j = 0; j < PJ_ARRAY_SIZE(sip_config.call[i].transport); ++j) {
			/* Repeat binding media socket to next port when fails to bind
			* * to current port number.
			* */
			int retry;
			sip_config.call[i].media_index = j;
			for (retry = 0; retry<100; ++retry, rtp_port += 2) {
				status = pjmedia_transport_udp_create3(sip_config.g_med_endpt, AF, NULL, NULL,
					rtp_port, 0,
					&sip_config.call[i].transport[j]);
				if (status == PJ_SUCCESS) {
					rtp_port += 2;
					/*
					*  Get socket info (address, port) of the media transport. We will
					*  need this info to create SDP (i.e. the address and port info in
					*  the SDP).
					*  */
					pjmedia_transport_info_init(&sip_config.call[i].tpinfo[j]);
					pjmedia_transport_get_info(sip_config.call[i].transport[j], &sip_config.call[i].tpinfo[j]);
					PJ_LOG(3, (THIS_FILE, "create media TP for call %d success!", i));
					break;
				}
			}
		}
		if (status != PJ_SUCCESS) {
			app_perror(THIS_FILE, "Unable to create media transport", status);
			goto err;
		}
	}
}

