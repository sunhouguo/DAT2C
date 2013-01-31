#pragma once
#include "protocol.h"

namespace Protocol {

	class CH103_MMP_CK :
		public CProtocol
	{
	public:
		CH103_MMP_CK(boost::asio::io_service & io_service);
		virtual ~CH103_MMP_CK(void);

		virtual int LoadXmlCfg(std::string filename);
		virtual void SaveXmlCfg(std::string filename);

	protected:
		virtual int CheckFrameHead(unsigned char * buf,size_t & exceptedBytes);
		virtual int CheckFrameTail(unsigned char * buf,size_t exceptedBytes);
		virtual int ParseFrameBody(unsigned char * buf,size_t exceptedBytes);

		virtual int AssembleFrameHead(size_t bufIndex, unsigned char * buf, CCmd & cmd);
		virtual int AssembleFrameBody(size_t bufIndex, unsigned char * buf, CCmd & cmd);
		virtual int AssembleFrameTail(size_t bufBegin, size_t bufIndex, unsigned char * buf, CCmd & cmd);

		virtual int InitProtocol();
		virtual void UninitProtocol();

	private:
		void InitObjectIndex();
		//void InitDefaultStartAddr();
		//void InitDefaultFrameElem();
		//void InitFrameLocation(size_t FrameHead);
		void InitDefaultTimeOut();
		void InitDefaultTimer(boost::asio::io_service & io_service);

		void handle_timerCallAllData(const boost::system::error_code& error,share_commpoint_ptr point);
		void handle_timerSynTime(const boost::system::error_code& error,share_commpoint_ptr point);
		void handle_timerYkSel(const boost::system::error_code& error,share_commpoint_ptr point,size_t yk_no);

		void ResetTimerCallAllData(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerSynTime(share_commpoint_ptr point,bool bContinue = true,unsigned short val = 0);
		void ResetTimerYkSel(share_commpoint_ptr point,size_t yk_no,bool bContinue = false,unsigned short val = 0);

		int AssembleSynTime(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr, boost::posix_time::ptime time);
		int AssembleCallAllData(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr);
		int AssembleYKSel(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char yk_obj, unsigned char yk_code);
		int AssembleYKExe(size_t bufIndex, unsigned char * buf, share_terminal_ptr terminalPtr,unsigned char yk_obj, unsigned char yk_code);

		int ParseAllData(unsigned char * buf, share_terminal_ptr terminalPtr);
		int ParseAllEvent(unsigned char * buf, share_terminal_ptr terminalPtr);
		int ParseYkSelCon(unsigned char * buf, share_terminal_ptr terminalPtr);

		int TransYkCmdToPara(int yk_no,typeYktype yk_type,unsigned char & yk_obj,unsigned char & yk_code);
		int TransYkParaToCmd(unsigned char yk_obj,unsigned char yk_code,int & yk_no,typeYktype & yk_type);

	private:
		enum H103_MMP_CKPara
		{
			BroadCastAddr = 0xff,

			DEFAULT_timeOutCallAllData = 3,
			MIN_timeOutCallAllData = 1,
			DEFAULT_timeOutSynTime = 1800,
			MIN_timeOutSynTime = 360,
			DEFAULT_timeOutYkSel = 10,
		};

		static size_t H103ObjectCounter_;

		unsigned short timeOutCallAllData_;
		timerPtr timerCallAllData_;
		unsigned short timeOutSynTime_;
		timerPtr timerSynTime_;
		unsigned short timeOutYkSel_;
		timerPtr timerYkSel_;
		unsigned char conCode;
	};

}; //namespace Protocol 
