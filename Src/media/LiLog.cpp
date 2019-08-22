#include "LiLog.h"
#include <vector>
//using namespace std;

using namespace erizo;

CLiLog::CLiLog()
{
	InitLiLog();
}

CLiLog::~CLiLog()
{
	//m_ofs.close();
	if (m_fp)
	fclose(m_fp);
}

static std::string LiLSplit(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		if (!token.empty()) {
			tokens.push_back(token);
		}
	}

	if (!tokens.empty())
		return tokens[tokens.end() - tokens.begin() - 1];
	return "";
}


int CLiLog::ToLiLog(void* vptr, std::string strDe, std::string strFe, std::string strFc, int nLine, std::string strInfo)
{
/*	m_ofs.write(strInfo.c_str(), strInfo.length());
	m_ofs.flush();*/
	//fprintf(m_fp, "=>[%p-%s-%s-%s-%d]", this, __DATE__, __FILE__, __FUNCTION__, __LINE__);
	//printf("=>[%p-%s-%s-%s-%d]", vptr, strDe.c_str(), LiLSplit(strFe, '\\').c_str(), strFc.c_str(), nLine);
	if (!m_fp)
		return -1;
	fprintf(m_fp, "=>[%p-%s-%s-%s-%d]", vptr, strDe.c_str(), LiLSplit(strFe, '\\').c_str(), strFc.c_str(), nLine);
	fwrite(strInfo.c_str(), strInfo.length(), 1, m_fp);
	fprintf(m_fp, "\n");
	fflush(m_fp);
	return strInfo.length();
}

int CLiLog::ToLiLog(std::string strInfo)
{
	return 0;
}

int CLiLog::InitLiLog()
{
	/*m_ofs.open("lilog.log", ios::out);
	if (m_ofs.is_open())
		return 1;*/
	m_fp = fopen("lilog.log", "wt");
	if (m_fp != NULL)
		return 1;
	return 0;
}
