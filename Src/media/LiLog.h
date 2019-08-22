#ifndef _LI_LOG_H_
#define _LI_LOG_H_

//#include <fstream>

#define LIL_LOG(p, p1, info) \
		p->ToLiLog(p1, __DATE__, __FILE__, __FUNCTION__, __LINE__ ,info)

namespace erizo {
	class CLiLog {
	public:
		CLiLog();
		virtual ~CLiLog();

		int ToLiLog(void* vptr, std::string strDe, std::string strFe, std::string strFc, int nLine, std::string strInfo);

		int ToLiLog(std::string strInfo);

		int InitLiLog();

	private:
		//std::ofstream m_ofs;
		FILE * m_fp;
	};
}

#endif