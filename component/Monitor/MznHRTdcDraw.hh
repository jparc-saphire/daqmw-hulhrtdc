#ifndef MznHRTdcDraw_h
#define MznHRTdcDraw_h

#include <string>

class THttpServer;

class MznHRTdcDraw
{
public:
  MznHRTdcDraw(const char* name);
  MznHRTdcDraw(const MznHRTdcDraw&)            = delete;
  MznHRTdcDraw& operator=(const MznHRTdcDraw&) = delete;
  MznHRTdcDraw(MznHRTdcDraw&&)                 = delete;
  MznHRTdcDraw& operator=(MznHRTdcDraw&&)      = delete;
  ~MznHRTdcDraw()                         = default;

  void Fill(const MZN_HRTDC::Data_t& cont,bool isupdate);
  void Initialize(THttpServer* server,
		  const std::string& name="mzntdc0");
  void Reset();

private:
  std::string fName;
};
#endif
