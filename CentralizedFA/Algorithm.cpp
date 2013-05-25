#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/exception/all.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <stack>
#include "Algorithm.h"
#include "OutLetSwitch.h"
#include "../FileSystem/Markup.h"
#include "../Protocol/Protocol.h"
#include "../FileSystem/Log.h"
#include "../FileSystem/LogFactory.h"
#include "../DataBase/DAOperate.h"
#include "../DataBase/YxPoint.h"

namespace CentralizedFA {

using namespace std;

const string strDA = "DA";
const string strSwitchNode = "SwitchNode";
const string strLogName = "LogName";
const string strInputYx = "InputYx";
const string strOutputYx = "OutputYx";
const string strPolar = "Polar";
const string strWaitYxTime = "WaitYxTime";
const string strWaitYkTime = "WaitYkTime";

//const string strSignResetTime = "SignResetTime";
//const string strOverLoadI  = "OverLoadChannel1";//定时记录故障前电源侧电流大小
//const string strOverLoadII = "OverLoadChannel2";//定时记录故障前电源侧电流大小
//const string strCheckOverLoad = "CheckOverLoad";//预判恢复隔离开关过负荷 投退
//const string strOverLoadValue = "OverLoadValue";

const string strCheckFADisable = "CheckFADisable";

typedef CMyBTreeNode<share_switchnode_ptr> SwitchTreeNode;

const unsigned char OutYx_FA_Enable = 0;         //策略FA投入
const unsigned char OutYx_FA_MulFault = 1;      //FA多重故障
//const unsigned char OutYx_FA_OverLoadAll =2;     //过负荷

CCentralizedDA::CCentralizedDA(/*boost::asio::io_service & io_service,*/std::string id,DataBase::CDAOperate & da_op)
						:id_(id),
						da_op_(da_op)
						//io_service_(io_service),
						//TimerSignReset_(io_service),
						//TimerGetLoad_(io_service)
{
	thread_run_ = false;

	//SignResetTime_=SIGN_RESET_TIME_DEFAULT;

	//CheckOverLoad_=0;
	//bCheckOverLoad_=false;
	//OverLoadValue_=6000;

	CheckFADisable_=false;
	//YcOverLoadI_=0;
	//YcOverLoadII_=0;

	//YcOverLoadValue1_=0;
	//YcOverLoadValue2_=0;

	//bSwitchRefuseSign_=false;
	//bDisableOutLed_=true;

	bReset_ = false;

	WaitYxTime_=500;//ms
	WaitYkTime_=5;//ms

	log_.reset();
}

CCentralizedDA::~CCentralizedDA(void)
{
}

int CCentralizedDA::AddLog(std::string val)
{
#ifdef Debug_DA
	std::cout<<val;
#endif

	if (log_)
	{
		return log_->AddRecordWithSynT(val);
	}

	return -1;
}

std::string CCentralizedDA::getID()
{
	return id_;
}

int CCentralizedDA::ConnectSubAlgorithmSig(CmdRecallSignalType & sig)
{
	SubAlgorithmSigConnection_ = sig.connect(boost::bind(&CCentralizedDA::ProcessAlgorithmSig,this,_1,_2,_3,_4));

	return 0;
}
int CCentralizedDA::DisconnectSubAlgorithmSig()
{
	SubAlgorithmSigConnection_.disconnect();

	return 0;
}

int CCentralizedDA::InitAlgorithm()
{
	return LoadXmlCfg(id_);
}

void CCentralizedDA::UninitAlgorithm()
{

}

share_switchnode_ptr CCentralizedDA::getWitchNodeByID(typeID val)
{
	for (std::vector<share_switchnode_ptr>::iterator it = OriSwitchNodes_.begin();it != OriSwitchNodes_.end();it++)
	{
		if ((*it)->getSwitchID() == val)
		{
			return *it;
		}
	}

	return share_switchnode_ptr();
}

int CCentralizedDA::BuildLeafQueue(queue<share_switchnode_ptr> & leafQueue)
{
	if (!tree_.IsEmpty())
	{
		if (tree_.IsLeaf())
		{
			leafQueue.push(tree_.GetCurrentData());
		}

		SwitchTreeNode * curPtr = tree_.GetCurrentPointer();
		bool bRet = tree_.FirstChild();
		while(bRet)
		{
			BuildLeafQueue(leafQueue);
			bRet = tree_.NextSibling();
		}
		tree_.SetCurrentPointer(curPtr);
	}

	return 0;
}

int CCentralizedDA::BuildForest()
{
	if(tree_.Root())                 //删除树
	{
		tree_.RemovesubTree();
	}

	//清空输出接点
	faultSet_.clear();
	ykCloseSet_.clear();
	ykOpenSet_.clear();
	outLetswitchSet_.clear();

	if (OriSwitchNodes_.size() == 0)
	{
		return -1;
	}

	tree_.BuildRoot(share_switchnode_ptr()); //建立一个空结点作为所有树的总根，将森林转换为一棵二叉树

	queue<share_switchnode_ptr> searchQueue;
	vector<share_switchnode_ptr> dealSwitchNodes = OriSwitchNodes_;

	for (vector<share_switchnode_ptr>::iterator it = dealSwitchNodes.begin(); it != dealSwitchNodes.end();) //找到源类型的开关结点，作为树根
	{
		if ((*it)->getbPowerNode()) //如果是源结点，插入为空根的子女
		{
			if (tree_.Root()) //current指针指向空树根
			{
				tree_.InsertChild(*it);

				std::ostringstream ostr;
				ostr<<"Power Node = "<<(*it)->getSwitchID()<<std::endl;
				AddLog(ostr.str());

				if ((*it)->getSwitchPosition())
				{
					std::ostringstream ostr;
					ostr<<"Search Node Push "<<(*it)->getSwitchID()<<std::endl;
					AddLog(ostr.str());

					searchQueue.push(*it); //如果源点开关位置为合位，将源结点加入搜索队列
				}

				it = dealSwitchNodes.erase(it); //从开关队列中取出处理过的结点
				continue;
			}
		}

		it++;
	}

	while(!searchQueue.empty()) //遍历搜索队列
	{
		share_switchnode_ptr node = searchQueue.front();
		
		if (tree_.Find(node)) //current指针指向当前结点
		{
			for (vector<share_switchnode_ptr>::iterator it = dealSwitchNodes.begin(); it != dealSwitchNodes.end();) 
			{
				if((*it)->CheckNodebLinked(node->getSwitchID())) //找出与当前结点有连接关系且未处理过的结点
				{
					tree_.InsertChild(*it); //连接结点加为当前结点的孩子
					if ((*it)->getSwitchPosition())
					{
						searchQueue.push(*it);

						std::ostringstream ostr;
						ostr<<"Search Node Push "<<(*it)->getSwitchID()<<std::endl;
						AddLog(ostr.str());
					}

					it = dealSwitchNodes.erase(it);
					continue;
				}

				it++;
			}
		}

		searchQueue.pop();
	}

	queue<share_switchnode_ptr> leafQueue;
	if (tree_.Root()) //current指针指向空树根
	{
		BuildLeafQueue(leafQueue);  //遍历树的所有结点，将树叶结点加入LeafQueue队列
	}

	while(!leafQueue.empty()) //遍历所有树叶结点
	{
		share_switchnode_ptr node = leafQueue.front();

		if (!node->getSwitchPosition()) //如果某树叶结点开关状态为分
		{
			if (tree_.Find(node))
			{
				SwitchTreeNode * srcLinkPtr = tree_.GetCurrentPointer(); //保存当前叶结点的指针

				share_switchnode_ptr parent;
				vector<share_switchnode_ptr> brotherSet;
				if (tree_.Parent()) //指针指向node结点的父结点
				{
					parent = tree_.GetCurrentData(); //记录node叶结点父结点的数据
					bool bRet = tree_.FirstChild(); //指针指向node结点父结点的第一个子女结点
					while(bRet)
					{
						brotherSet.push_back(tree_.GetCurrentData()); //保存node结点所有兄弟结点的数据
						bRet = tree_.NextSibling();
					}
				}
				else
				{
					parent.reset();
				}

				SwitchTreeNode * dstLinkPtr = NULL;
				for (vector<share_switchnode_ptr>::iterator it = OriSwitchNodes_.begin();it != OriSwitchNodes_.end();it++)
				{ 
					if(node->CheckNodebLinked((*it)->getSwitchID())) //遍历所有与当前结点有连接关系的结点
					{
						if (parent != NULL) //当前结点有父结点
						{
							if(*it != parent) //排除该父节点
							{
								bool isBrother = false;
								for (vector<share_switchnode_ptr>::iterator mt = brotherSet.begin();mt != brotherSet.end();mt++)
								{
									if (*mt == *it) //该连接结点为兄弟结点,不符合要求
									{
										isBrother = true;
										break;
									}
								}

								if (!isBrother) //该连接结点不是兄弟结点才判断衍生枝
								{
									if (tree_.Find(*it))
									{
										if (dstLinkPtr) //若node结点有一个以上连接点，取其中为父结点的点为衍生枝
										{
											SwitchTreeNode * dstLinkBackup = tree_.GetCurrentPointer();
											while(!tree_.IsRoot())
											{
												tree_.Parent();
												if (dstLinkPtr == tree_.GetCurrentPointer())
												{
													break;
												}
											}

											if (tree_.IsRoot())
											{
												dstLinkPtr = dstLinkBackup;
											}
										}
										else //若node结点只有一个连接点，将该点作为衍生枝
										{
											dstLinkPtr = tree_.GetCurrentPointer();
										}
									}
								}
							}
						}
						else //当前结点没有父结点
						{
							if (tree_.Find(*it)) //取多个与当前结点有连接关系的结点中非父的结点作为当前结点的衍生枝
							{
								if (dstLinkPtr)
								{
									SwitchTreeNode * dstLinkBackup = tree_.GetCurrentPointer();
									while(!tree_.IsRoot())
									{
										tree_.Parent();
										if (dstLinkPtr == tree_.GetCurrentPointer())
										{
											break;
										}
									}

									if (tree_.IsRoot())
									{
										dstLinkPtr = dstLinkBackup;
									}
								}
								else //若node结点只有一个连接点，将该点作为衍生枝
								{
									dstLinkPtr = tree_.GetCurrentPointer();
								}
							}
						}
					}
				}
				
				if (dstLinkPtr)
				{
					tree_.SetLink(srcLinkPtr,dstLinkPtr); //为两个结点建立衍生枝
				}
			}
		}

		leafQueue.pop();
	}

	return 0;
}

int CCentralizedDA::CheckFault()
{
	bool bRet = false;

	if (tree_.Root()) //current指针指向森林的空树根
	{
		bRet = tree_.FirstChild(); //current指针指向第一个孩子
	}

	queue<share_switchnode_ptr> faultPowerQueue;
	queue<share_switchnode_ptr> resumeQueue;
	while(bRet) //循环遍历空树根的所有子女结点，即遍历所有电源结点
	{
		share_switchnode_ptr node = tree_.GetCurrentData();

		if(node)
		{
			if (node->getSwitchProtected()) //源点的保护动作信号启动
			{
				share_outletswitch_ptr outlet = node->getOutLetSwitch();
				if (outlet && outlet->AllowYx()) //如果该源点前有一个出线开关,并且该出线开关需要判断遥信
				{
					if ((!outlet->getSwitchPosition()) && (outlet->getSwitchProtected())) //判断出线开关的保护动作和开关位置信号
					{
						faultPowerQueue.push(node);
					}
				}
				else
				{
					faultPowerQueue.push(node);
				}

				faultSet_.push_back(node); //将检测到的故障结点记录下来
			}
			else //如果源点没有故障信号，而出口断路器去动作的情况
			{
				share_outletswitch_ptr outlet = node->getOutLetSwitch();
				if (outlet && outlet->AllowYx()) //如果该源点前有一个出线开关,并且该出线开关需要判断遥信
				{
					if ((!outlet->getSwitchPosition()) && (outlet->getSwitchProtected())) //判断出线开关的保护动作和开关位置信号，如果出线开关动作而源点自身没有故障信号，说明故障在源点和出现开关之间
					{
						outlet->setAllowYk(false);
						bReset_ = true;

						ykOpenSet_.push_back(node);
						resumeQueue.push(node);
					}
				}
			}
		}

		bRet = tree_.NextSibling();
	}

	int result = 0;

	if (!faultPowerQueue.empty())
	{
		result = SearchFaultNode(faultPowerQueue);
	}

	if (!resumeQueue.empty())
	{
		result = Resume(resumeQueue);
	}

	return result;
}

int CCentralizedDA::SearchFaultNode(std::queue<share_switchnode_ptr> & powerQueue)
{
	queue<share_switchnode_ptr> insulateQueue;

	while(!powerQueue.empty())
	{
		share_switchnode_ptr powerNode = powerQueue.front();
		share_switchnode_ptr node = powerQueue.front();

		if (tree_.Find(node)) //指针指向node结点
		{
			queue<SwitchTreeNode *> scopeQueue; //广度优先遍历子树时用的存放子女结点的队列
			if (!tree_.IsEmpty())
			{
				SwitchTreeNode * curPtr = tree_.GetCurrentPointer(); //保存当前结点指针
				scopeQueue.push(curPtr);

				while(!scopeQueue.empty()) //广度优先遍历子树
				{
					SwitchTreeNode * tmpPtr = scopeQueue.front();

					tree_.SetCurrentPointer(tmpPtr);
					share_switchnode_ptr parentNode = tree_.GetCurrentData();
					share_switchnode_ptr node = tree_.GetCurrentData();

					bool parentNoFault = false;
					bool parentNeedInsulate = false;
					if (node->getSwitchProtected()) //当前结点发现故障遥信
					{
						if (tree_.IsLeaf()) //若当前结点为叶结点，该结点即为故障终端结点之一
						{
							insulateQueue.push(node); //加入隔离队列

							share_outletswitch_ptr outletswitch = powerNode->getOutLetSwitch(); 
							if (outletswitch) //如果该故障结点的源点前有一个出线开关
							{
								if (outletswitch->AllowYk()) //该出现开关允许遥控
								{
									AddOutLetSwitch(outletswitch); //将该出线开关动作加入待合闸的出线开关队列
								}
							}

							tree_.FirstChild();

							parentNeedInsulate = false;
						}
						else //若当前结点为非叶结点
						{
							tree_.FirstChild(); //准备遍历它的子女结点

							parentNeedInsulate = true;//遍历子女结点后，可能判断出当前结点也为故障终端结点
						}

						parentNoFault = false;

						faultSet_.push_back(node); //将检测到的故障结点记录下来
					}
					else //当前结点未发现故障遥信
					{
						tree_.FirstChild();

						parentNeedInsulate = false;
						parentNoFault = true; //遍历子女结点后，可能判断出这是一次遥信误报
					}

					while(!tree_.IsEmpty()) //遍历当前结点的所有子女结点
					{
						scopeQueue.push(tree_.GetCurrentPointer());
						share_switchnode_ptr node = tree_.GetCurrentData();
						if (node->getSwitchProtected()) //某子女结点发现故障遥信
						{
							parentNeedInsulate = false; //该子女结点的父结点不可能为故障终端结点
							if (parentNoFault) //该子女结点的父结点未发现故障遥信
							{
								cerr<<"故障遥信误报"<<endl;
								return -1;  //故障遥信误报，不采取任何隔离或恢复措施
							}

							faultSet_.push_back(node); //将检测到的故障结点记录下来
						}

						tree_.NextSibling();
					}

					if (parentNeedInsulate) //遍历所有子女结点后；判断出当前结点为故障终端结点
					{
						insulateQueue.push(parentNode);

						share_outletswitch_ptr outletswitch = powerNode->getOutLetSwitch(); 
						if (outletswitch) //如果该故障结点的源点前有一个出线开关
						{
							if (outletswitch->AllowYk()) //该出现开关允许遥控
							{
								AddOutLetSwitch(outletswitch); //将该出线开关动作加入待合闸的出线开关队列
							}
						}
					}

					scopeQueue.pop();
				}

				tree_.SetCurrentPointer(curPtr);
			}
		}

		powerQueue.pop();
	}

	int ret = 0;

	if (!insulateQueue.empty())
	{
		ret = Insulate(insulateQueue);
	}

	return ret;
}

int CCentralizedDA::Insulate(queue<share_switchnode_ptr> & insulateQueue)
{
	queue<share_switchnode_ptr> resumeQueue;

	while(!insulateQueue.empty())
	{
		share_switchnode_ptr node = insulateQueue.front();

		if (node->getSwitchPosition()) //如果开关是合闸位置
		{
			ykOpenSet_.push_back(node); //将该开关加入分闸队列
		}
		
		if(tree_.Find(node))
		{
			bool bRet = tree_.FirstChild();
			while(bRet) //遍历该故障终端结点的子女结点
			{
				share_switchnode_ptr node = tree_.GetCurrentData();
				if (node->getSwitchPosition()) //如果开关是合闸位置
				{
					ykOpenSet_.push_back(node); //将该开关加入分闸队列
					resumeQueue.push(node);  //将该子女结点加入恢复队列
				}

				bRet = tree_.NextSibling();
			}
		}

		insulateQueue.pop();
	}

	int ret = 0;

	if (!resumeQueue.empty())
	{
		ret = Resume(resumeQueue);
	}

	return ret;
}

int CCentralizedDA::Resume(queue<share_switchnode_ptr> & resumeQueue)
{
	typedef queue<share_switchnode_ptr> share_switchnode_queue;
	vector<share_switchnode_queue> resume_set;

	while(!resumeQueue.empty())
	{
		share_switchnode_ptr node = resumeQueue.front();
		
		if (tree_.Find(node))
		{
			share_switchnode_ptr rootNode = node; //记录以resume队列中结点为树根的子树的树根结点
			SwitchTreeNode * curPtr = tree_.GetCurrentPointer(); //保存当前结点指针

			queue<share_switchnode_ptr> tmp_queue;
			stack<SwitchTreeNode *> tmp_stack;
			do 
			{
				while(!tree_.IsEmpty())
				{
					if (tree_.HasLinkNode()) //如果该点有衍生枝
					{
						share_switchnode_ptr node = tree_.GetCurrentData();
						share_switchnode_ptr linknode = tree_.GetLinkData();

						if (node != rootNode) //如果有衍生枝的该结点不是子树的根结点，即不是resume队列中的结点
						{
							//判断该结点和其衍生结点的遥信位置，二者遥信位置相反则满足条件，将分位的结点放入待合闸队列
							if (node->getSwitchPosition() && (!linknode->getSwitchPosition()))
							{
								tmp_queue.push(linknode);
							}
							else if ((!node->getSwitchPosition()) && linknode->getSwitchPosition())
							{
								tmp_queue.push(node);
							}
						}
						else //如果有衍生枝的该结点是子树的根结点，即是resume队列中的结点
						{
							if (!linknode->getSwitchPosition()) //只有其衍生结点位于分为才满足条件，将衍生结点放入待合闸队列
							{
								tmp_queue.push(linknode);
							}
						}
					}

					tmp_stack.push(tree_.GetCurrentPointer());
					tree_.FirstChild();
				}

				while(tree_.IsEmpty() && (!tmp_stack.empty()))
				{
					tree_.SetCurrentPointer(tmp_stack.top());
					tmp_stack.pop();
 
					if (tree_.GetCurrentData() != rootNode) //如果深度优先搜索尚未定位到子树的根结点，即resume队列中的结点，继续搜索这棵子树
					{
						tree_.NextSibling();
					}
					else //如果深度优先搜索已经定位到子树的根结点，即resume队列中的结点，则本棵子树的搜索结束。准备搜索以resume队列中下一个结点为树根的子树
					{
						tree_.SetCurrentPointer(NULL);
					}
				}
			}
			while (!tree_.IsEmpty());

			tree_.SetCurrentPointer(curPtr);//将指针恢复到最初位置

			if (tmp_queue.size() == 1) //如果resume队列中的某结点只有一条衍生枝
			{
				ykCloseSet_.push_back(tmp_queue.front());
				tmp_queue.pop();
			}
			else if (tmp_queue.size() > 1) //如果resume队列中的某结点有多条衍生枝
			{
				resume_set.push_back(tmp_queue); //将衍生枝结点送入备选方案集合中
			}
		}

		resumeQueue.pop();
	}

	if (resume_set.size() > 0) //如果resume队列中某结点有一个以上联络开关，以排他算法选择一个开关
	{
		bool bAddYkCloseQueue=true;
		while(bAddYkCloseQueue)
		{
			bAddYkCloseQueue = false; //默认本while循环只执行一次

			for (vector<share_switchnode_queue>::iterator it = resume_set.begin();it != resume_set.end();) //遍历所有的有多个衍生枝的结点
			{
				for(size_t j=0;j<(*it).size();j++) //遍历某结点的衍生枝一轮
				{
					//vector<share_switchnode_ptr> ykCloseQueueTmp = ykCloseQueue_; //将YKCloseQueue队列做个Copy
					share_switchnode_ptr ykNode = (*it).front();

					bool bUsed = false;
					for (vector<share_switchnode_ptr>::iterator mt = ykCloseSet_.begin(); mt != ykCloseSet_.end();mt++)//遍历YKCloseQueue队列
					{
						if (ykNode == *mt) //如果某衍生结点已经被其他方案使用
						{
							(*it).pop(); //从备选方案中将该衍生结点删除
							bUsed = true;
							break;
						}
					}

					if (!bUsed) //如果该衍生结点没有被其他任何方案使用
					{
						(*it).pop();
						(*it).push(ykNode); //将该衍生结点放入备选方案队列的最后位置,这样不在被检查第二次。
					}
				}

				if ((*it).size() <= 1) //如果某备选方案中衍生枝的个数已经小于1
				{
					if ((*it).size() > 0) 
					{
						ykCloseSet_.push_back((*it).front()); //将该衍生结点加入YKCloseQueue队列。
						(*it).pop();

						bAddYkCloseQueue = true;
					}

					it = resume_set.erase(it);
					continue;
				}

				it++;
			}
		}

		for (vector<share_switchnode_queue>::iterator it = resume_set.begin();it != resume_set.end();) //while循环结束，再检查一遍剩下的备选方案
		{
			//从多个衍生结点中选择第一个加入YKCloseQueue队列，作为该备选方案的合闸结点
			ykCloseSet_.push_back((*it).front());
			it = resume_set.erase(it);
		}
	}

	return 0;
}

int CCentralizedDA::AddOutLetSwitch(share_outletswitch_ptr outlet)
{
	for (vector<share_outletswitch_ptr>::iterator it = outLetswitchSet_.begin(); it != outLetswitchSet_.end(); it++)
	{
		if ((*it) == outlet)
		{
			return 1;
		}
	}

	outLetswitchSet_.push_back(outlet);

	return 0;
}

void CCentralizedDA::ClearAlgorithmData()
{
	if (tree_.Root())
	{
		tree_.RemovesubTree();
	}

	faultSet_.clear();
	OriSwitchNodes_.clear();
	ykCloseSet_.clear();
	ykOpenSet_.clear();
	outLetswitchSet_.clear();
}

int CCentralizedDA::LoadXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;

	if (!xml.Load(filename))
	{
		std::cerr<<"导入xml配置文件"<<filename<<"失败！"<<std::endl;

		return -1;
	}

	ClearAlgorithmData();

	xml.ResetMainPos();
	xml.FindElem(); //root
	xml.IntoElem();  //enter root

	xml.ResetChildPos();
	if (xml.FindElem(strLogName))
	{
		std::string strTmp = xml.GetData();
		boost::trim(strTmp);
		log_.reset(FileSystem::CLogFactory::CreateLog(strTmp,FileSystem::strTextLog));
	}

	xml.ResetMainPos();
	int count = 0;
	while (xml.FindElem(strSwitchNode))
	{
		share_switchnode_ptr switchPtr(new CSwitchNode(da_op_));

		xml.IntoElem();  //enter strSwitchNode

		try
		{
			switchPtr->LoadXmlCfg(xml);
		}
		catch(PublicSupport::dat2def_exception & e)
		{
			switchPtr.reset();

			std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
			if (strPtr)
			{
				std::ostringstream ostr;
				ostr<<"初始化第"<<count<<"个"<<strSwitchNode<<"失败，";

				std::string const * strPtrMiddle = boost::get_error_info<errinfo_middletype_name>(e);
				if (strPtrMiddle)
				{
					ostr<<" item info:"<<(*strPtrMiddle);
				}

				int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
				if (indexPtr)
				{
					ostr<<" data no:"<<(*indexPtr)<<"，";
				}

				ostr<<"error info:"<<(*strPtr)<<"。";
				ostr<< "这个结点的配置将被忽略,并退出DA"<<std::endl;
				AddLog(ostr.str());
			}

			return -1;
		}

		xml.OutOfElem(); //out strSwitchNode

		OriSwitchNodes_.push_back(switchPtr);

		count++;
	}

	xml.ResetMainPos();
	while(xml.FindElem(strInputYx))
	{
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			size_t index = boost::lexical_cast<size_t>(strTmp);

			strTmp = xml.GetAttrib(strPolar);
			bool polar = true;
			if (boost::iequals(strTmp,strboolFalse))
			{
				polar = false;
			}

			strTmp = xml.GetAttrib(strOutputYx);
			int out = boost::lexical_cast<int>(strTmp);

			stIn val(index,out,polar);
			input_set_.push_back(val);
		}
		catch(boost::bad_lexical_cast & e)
		{
			ostringstream ostr;
			ostr<<"非法的输入遥信参数:"<<e.what();
			cerr<<ostr.str();
		}
	}

	xml.ResetMainPos();
	while(xml.FindElem(strOutputYx))
	{
		try
		{
			string strTmp = xml.GetData();
			boost::algorithm::trim(strTmp);
			size_t index = boost::lexical_cast<size_t>(strTmp);
			strTmp = xml.GetAttrib(strPolar);
			bool polar = true;
			if (boost::iequals(strTmp,strboolFalse))
			{
				polar = false;
			}
			stOut val(index,polar);
			output_set_.push_back(val);
		}
		catch(boost::bad_lexical_cast & e)
		{
			ostringstream ostr;
			ostr<<"非法的输出遥信参数:"<<e.what();
			cerr<<ostr.str();
		}
	}

	//xml.ResetMainPos();
	//if(xml.FindElem(strSignResetTime))
	//	{
	//		std::string strTmp = xml.GetData();
	//		boost::algorithm::trim(strTmp);
	//		try
	//		{
	//			unsigned short timeout = boost::lexical_cast<unsigned short>(strTmp);
	//			SignResetTime_=timeout;
	//		}
	//		catch(boost::bad_lexical_cast& e)
	//		{
	//			std::ostringstream ostr;
	//			ostr<<e.what();
	//		}
	//	}

	//xml.ResetMainPos();
	//if (xml.FindElem(strOverLoadI))
	//{
	//	try
	//	{
	//		string strTmp = xml.GetData();
	//		boost::algorithm::trim(strTmp);
	//		unsigned short value = boost::lexical_cast<size_t>(strTmp);
	//		YcOverLoadI_=value;
	//	}
	//	catch(boost::bad_lexical_cast& e)
	//	{
	//		ostringstream ostr;
	//		ostr<<"非法的遥测点号参数:"<<e.what();

	//		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
	//	}
	//}
	//xml.ResetMainPos();
	//if (xml.FindElem(strOverLoadII))
	//{
	//	try
	//	{
	//		string strTmp = xml.GetData();
	//		boost::algorithm::trim(strTmp);
	//		unsigned short value = boost::lexical_cast<size_t>(strTmp);
	//		YcOverLoadII_=value;
	//	}
	//	catch(boost::bad_lexical_cast& e)
	//	{
	//		ostringstream ostr;
	//		ostr<<"非法的遥测点号参数:"<<e.what();

	//		throw PublicSupport::dat2def_exception()<<boost::errinfo_type_info_name(ostr.str());
	//	}
	//}

	//xml.ResetMainPos();
	//if (xml.FindElem(strCheckOverLoad))
	//{
	//	string strTmp = xml.GetData();
	//	boost::algorithm::trim(strTmp);
	//	if (boost::iequals(strTmp,strboolTrue))
	//	{
	//		bCheckOverLoad_ = true;
	//	}
	//	else
	//	{
	//		bCheckOverLoad_ = false;
	//	}
	//}

	xml.ResetMainPos();
	if (xml.FindElem(strCheckFADisable))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			int value = boost::lexical_cast<int>(strTmp);
			CheckFADisable_=value;
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}
	}

	xml.ResetMainPos();
	if(xml.FindElem(strWaitYxTime))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			int timeout = boost::lexical_cast<int>(strTmp);
			WaitYxTime_=timeout;
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}
	}


	xml.ResetMainPos();
	if(xml.FindElem(strWaitYkTime))
	{
		std::string strTmp = xml.GetData();
		boost::algorithm::trim(strTmp);
		try
		{
			int timeout = boost::lexical_cast<int>(strTmp);
			WaitYkTime_=timeout;
		}
		catch(boost::bad_lexical_cast& e)
		{
			std::ostringstream ostr;
			ostr<<e.what();
		}
	}


	//xml.ResetMainPos();
	//if(xml.FindElem(strOverLoadValue))
	//{
	//	std::string strTmp = xml.GetData();
	//	boost::algorithm::trim(strTmp);
	//	try
	//	{
	//		int value = boost::lexical_cast<int>(strTmp);
	//		OverLoadValue_=value;
	//	}
	//	catch(boost::bad_lexical_cast& e)
	//	{
	//		std::ostringstream ostr;
	//		ostr<<e.what();
	//	}
	//}


	xml.OutOfElem(); //out root

	return 0;
}

void CCentralizedDA::SaveXmlCfg(std::string filename)
{
	FileSystem::CMarkup xml;
	xml.SetDoc(strXmlHead);
	xml.SetDoc(strSubXsl);

	xml.AddElem(strDA);
	xml.IntoElem();//enter root

	if (log_)
	{
		xml.AddElem(strLogName,log_->getLogPath());
	}

	for (vector<share_switchnode_ptr>::iterator it = OriSwitchNodes_.begin(); it != OriSwitchNodes_.end();it++)
	{
		xml.AddElem(strSwitchNode);

		xml.IntoElem();  //enter strSwitchNode

		(*it)->SaveXmlCfg(xml);

		xml.OutOfElem(); //out strSwitchNode
	}

	for(std::vector<stIn>::iterator it = input_set_.begin();it != input_set_.end();it++)
	{
		std::string polar;
		if((*it).polar_)
		{
			polar = strboolTrue;
		}
		else
		{
			polar = strboolFalse;
		}

		xml.AddElem(strInputYx,(*it).index_);
		xml.AddAttrib(strPolar,polar);
		xml.AddAttrib(strOutputYx,(*it).out_);
	}

	for(std::vector<stOut>::iterator it = output_set_.begin();it != output_set_.end();it++)
	{
		std::string polar;
		if ((*it).polar_)
		{
			polar = strboolTrue;
		}
		else
		{
			polar = strboolFalse;
		}

		xml.AddElem(strOutputYx,(*it).index_);
		xml.AddAttrib(strPolar,polar);
	}

	xml.OutOfElem();//out root
	xml.Save(filename);//save xml file
}

int CCentralizedDA::OpenSwitch(share_switchnode_ptr node)
{
	if((node)->AllowYk())
	{
		int ret = (node)->OpenSwitch();
		if (ret)
		{
			std::ostringstream ostr;
			ostr<<"隔离开关"<<(node)->getSwitchID()<<"失败，退出本次DA"<<std::endl;
			AddLog(ostr.str());

			return ret;
		}

		return WaitYkResult(node,boost::posix_time::second_clock::local_time(),false);
	}

	return 0;
}

int CCentralizedDA::CloseSwitch(share_switchnode_ptr node)
{
	if(((node)->AllowYk())/*&&(!bSwitchRefuseSign_)*/)
	{
		int ret = (node)->CloseSwitch();
		if (ret)
		{
			std::ostringstream ostr;
			ostr<<"恢复联络开关"<<(node)->getSwitchID()<<"失败，退出本次DA"<<std::endl;
			AddLog(ostr.str());

			return ret;
		}

		return WaitYkResult(node,boost::posix_time::second_clock::local_time(),true);
	}

	return 0;
}

int CCentralizedDA::OpenOutlet(share_outletswitch_ptr node)
{
	return 0;
}

int CCentralizedDA::CloseOutlet(share_outletswitch_ptr node)
{
	if((node)->AllowYk())
	{
		int ret = (node)->CloseSwitch();
		if (ret)
		{
			std::ostringstream ostr;
			ostr<<"恢复出线开关"<<(node)->getSwitchID()<<"失败，退出本次DA"<<std::endl;
			AddLog(ostr.str());

			return ret;
		}

		return WaitYkResult(node,boost::posix_time::second_clock::local_time(),true);

	}

	return 0;
}

int CCentralizedDA::OutPutYk()
{
	for (vector<share_switchnode_ptr>::iterator it = ykOpenSet_.begin();it != ykOpenSet_.end();it++) //隔离故障
	{
		//(*it)->WriteOutputYx(OutYx_FA_Act,1,true);
		int ret = OpenSwitch(*it);
		if(ret)
		{
			return ret;
		}
        
		boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(10));
	} 

	for (vector<share_switchnode_ptr>::iterator it = ykCloseSet_.begin();it != ykCloseSet_.end();it++) //恢复供电
	{
		//(*it)->WriteOutputYx(OutYx_FA_Act,1,true);
		int ret = CloseSwitch(*it);
		if (ret)
		{
			return ret;
		}

		boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(10));
	}

	for (vector<share_outletswitch_ptr>::iterator it = outLetswitchSet_.begin();it != outLetswitchSet_.end();it++) //恢复出线开关
	{
		//(*it)->WriteOutputYx(OutYx_FA_Act,1,true);
		int ret = CloseOutlet(*it);
		if (ret)
		{
			return ret;
		}

		boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(10));
	}

	return 0;
}

int CCentralizedDA::WaitYkResult(share_outletswitch_ptr node,boost::posix_time::ptime begin_time,bool excepted_swichposition)
{
	boost::posix_time::ptime now_time = boost::posix_time::second_clock::local_time();
	if((now_time >= begin_time) && (now_time - begin_time <= boost::posix_time::seconds(wait_yxcon_timeout)))
	{
		boost::unique_lock<boost::mutex> uLock(yxMutex_,boost::try_to_lock);
		if (uLock.owns_lock())
		{
			boost::posix_time::time_duration wait_time = boost::posix_time::seconds(wait_yxcon_timeout) - (now_time - begin_time);

            boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(5));//10*500

			if(yxCond_.timed_wait(uLock,boost::get_system_time() + wait_time))
			{
				uLock.unlock();

				if (CheckInput())
				{
					std::ostringstream ostr;
					ostr<<"隔离开关"<<node->getSwitchID()<<"过程中，检测到输入遥信异常，退出本次DA"<<std::endl;
					AddLog(ostr.str());

					return -1;
				}

				if (CheckNewFaultNode())
				{
					std::ostringstream ostr;
					ostr<<"隔离开关"<<node->getSwitchID()<<"过程中，检测到新的故障，退出本次DA"<<std::endl;
					AddLog(ostr.str());

					FaMulFault(true);

					return -1;
				}


				if ((node)->getSwitchPosition() == excepted_swichposition)
				{
					std::ostringstream ostr;
					ostr<<"隔离开关"<<node->getSwitchID()<<"后收到开关位置遥信变位，本开关遥控完成，继续DA"<<std::endl;
					AddLog(ostr.str());

					//(node)->WriteOutputYx(OutYx_FA_Over,1,true);
					node->SwitchActOver();

					return 0;
				}
				else
				{
					return WaitYkResult(node,begin_time,excepted_swichposition);
				}
			}
			else
			{
				std::ostringstream ostr;
				ostr<<"隔离开关"<<node->getSwitchID()<<"后，等待超时未能收到开关位置遥信变位，退出本次DA"<<std::endl;
				AddLog(ostr.str());
                
				//bSwitchRefuseSign_=true;
				//(node)->WriteOutputYx(OutYx_FA_Refuse,1,true);//拒动
				node->SwitchActError(); //误动

				return -1;
			}
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"隔离开关"<<node->getSwitchID()<<"后不能取得DA线程锁权限，退出本次DA"<<std::endl;
			AddLog(ostr.str());

			return -1;
		}
	}
	else
	{
		std::ostringstream ostr;
		ostr<<"隔离开关"<<node->getSwitchID()<<"超时后，未能收到开关位置遥信变位，退出本次DA"<<std::endl;
		AddLog(ostr.str());

		//bSwitchRefuseSign_=true;
		//(node)->WriteOutputYx(OutYx_FA_Refuse,1,true);
		node->SwitchActError(); //误动

		return -1;
	}
}

void CCentralizedDA::ProcessAlgorithmSig(typeCmd cmdType,unsigned char ReturnCode,share_commpoint_ptr point,boost::any val)
{
	switch(cmdType)
	{
	case Protocol::COS_DATA_UP:
		{
			if (ReturnCode == RETURN_CODE_ACTIVE || ReturnCode == RETURN_CODE_CMDSEND)
			{
				boost::unique_lock<boost::mutex> uLock(yxMutex_,boost::try_to_lock);
				if (uLock.owns_lock())
				{
					yxCond_.notify_one();
				}
			}
		}
		break;

	case Protocol::SOE_DATA_UP:
		{
			if (ReturnCode == RETURN_CODE_ACTIVE || ReturnCode == RETURN_CODE_CMDSEND)
			{
				boost::unique_lock<boost::mutex> uLock(yxMutex_,boost::try_to_lock);
				if (uLock.owns_lock())
				{
					yxCond_.notify_one();
				}
			}
		}
		break;

	default:
		break;
	}
}

void CCentralizedDA::ResetCfg()
{
	if (bReset_)
	{
		bReset_ = false;
		
		InitAlgorithm();
	}
}

void CCentralizedDA::thread_fun_da()
{
	boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(2000));

    FaAct(true);

	while(thread_run_)
	{
		boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(4000));

		if (!CheckInput())
		{
			try
			{
				ResetCfg();

				BuildForest();

				CheckFault();

				OutPutYk();
			}
			catch(PublicSupport::dat2def_exception & e)
			{
				std::string const * strPtr = boost::get_error_info<boost::errinfo_type_info_name>(e);
				if (strPtr)
				{
					std::ostringstream ostr;

					std::string const * strPtrMiddle = boost::get_error_info<errinfo_middletype_name>(e);
					if (strPtrMiddle)
					{
						ostr<<" item info:"<<(*strPtrMiddle);
					}

					int const * indexPtr = boost::get_error_info<boost::errinfo_errno>(e);
					if (indexPtr)
					{
						ostr<<" data no:"<<(*indexPtr)<<"，";
					}

					ostr<<"error info:"<<(*strPtr)<<"。";
					ostr<< "DA过程失败，请检查配置"<<std::endl;

					AddLog(ostr.str());
				}
			}
		}
		else
		{
			std::ostringstream ostr;
			ostr<<"输入遥信检查失败，终止本次DA"<<std::endl;
			AddLog(ostr.str());
		}

		FaMulFault(false);

		boost::unique_lock<boost::mutex> uLock(yxMutex_);

		yxCond_.wait(uLock);

		uLock.unlock();
	}

	FaAct(false);
}

void CCentralizedDA::FaAct(bool act)
{
	if (act)
	{
		WriteOutputYx(OutYx_FA_Enable,1,true);
	}
	else
	{
		WriteOutputYx(OutYx_FA_Enable,0,true);
	}
}

void CCentralizedDA::FaMulFault(bool act)
{
	if (act)
	{
		WriteOutputYx(OutYx_FA_MulFault,1,true);
	}
	else
	{
		WriteOutputYx(OutYx_FA_MulFault,0,true);
	}
}

int CCentralizedDA::start(CmdRecallSignalType & sig)
{
	ConnectSubAlgorithmSig(sig);

	thread_run_ = true;
	boost::thread thrd(boost::bind(&CCentralizedDA::thread_fun_da,this));

	return 0;
}

void CCentralizedDA::stop()
{
	DisconnectSubAlgorithmSig();

	thread_run_ = false;
	yxCond_.notify_one();
}

bool CCentralizedDA::CheckInputYxSet()
{
	for (std::vector<stIn>::iterator it = input_set_.begin(); it != input_set_.end(); it++)
	{
		size_t yxindex = (*it).index_;
		bool bPolar = (*it).polar_;

		bool bSingleYx = (da_op_.getYxType(yxindex) == DataBase::single_yx_point);
		typeYxval yxval = da_op_.getYxVal(yxindex);
		bool bRet = false;

		if (bSingleYx)
		{
			if (bPolar)
			{
				bRet = (yxval == 0x00);
			}
			else
			{
				bRet = (yxval == 0x01);
			}
		}
		else
		{
			if (bPolar)
			{
				bRet = (yxval == 0x01);
			}
			else
			{
				bRet = (yxval == 0x10);
			}
		}

		if (!bRet && (*it).out_ > 0)
		{
			WriteInputYx(*it,1,true);

			return bRet;
		}
	}

	return true;
}

int CCentralizedDA::WriteInputYx(stIn st,typeYxval val ,bool bSingleType)
{
	if (st.out_ > 0)
	{
		if (!st.polar_)
		{
			if (bSingleType)
			{
				val = ((~val) & 0x01);
			}
			else
			{
				val = ((~val) & 0x03);
			}
		}
		
		da_op_.TrigCosEvent(st.out_,val,bSingleType);

		return 0;
	}

	return -1;
}

int CCentralizedDA::WriteOutputYx(int outputindex,typeYxval val,bool bSingleType /* = true*/)
{
	if (outputindex < 0 || outputindex >= (int)output_set_.size())
	{
		return -1;
	}

	if (!output_set_[outputindex].polar_)
	{
		if (bSingleType)
		{
			val = ((~val) & 0x01);
		}
		else
		{
			val = ((~val) & 0x03);
		}
	}

	da_op_.TrigCosEvent(output_set_[outputindex].index_,val,bSingleType);

	return 0;
}

int CCentralizedDA::CheckInput()
{
	if (!CheckInputYxSet())
	{
		return -1;
	}

	for(vector<share_switchnode_ptr>::iterator it = OriSwitchNodes_.begin(); it != OriSwitchNodes_.end(); it++)
	{
		if (!(*it)->CheckInputYxSet())
		{
			return -1;
		}
	}

	return 0;
}

bool CCentralizedDA::CheckFaultAlreadyExist(share_switchnode_ptr node)
{
	for (vector<share_switchnode_ptr>::iterator it = faultSet_.begin(); it != faultSet_.end();it++)
	{
		if ((*it) == node)
		{
			return true;
		}
	}

	return false;
}

bool CCentralizedDA::CheckNewFaultNode()
{
	for (vector<share_switchnode_ptr>::iterator it = OriSwitchNodes_.begin();it != OriSwitchNodes_.end();it++)
	{
		if ((*it)->getSwitchProtected())
		{
			if (!CheckFaultAlreadyExist(*it))
			{
				return true;
			}
		}
	}

	return false;
}

//bool CAlgorithmDA::CheckFaultSwitch(bool excepted_swichposition)
//{
//	for (vector<share_switchnode_ptr>::iterator it = OriSwitchNodes_.begin();it != OriSwitchNodes_.end();it++)
//	{
//		if ((*it)->getSwitchPosition()==excepted_swichposition)
//		{
//				return true;
//		}
//	}
//
//	return false;
//}



//bool CAlgorithmDA::ResetOutputYx(typeYxval val,bool bSingleType)
//{
//	int count = 0;
////	for (std::vector<stInOut>::iterator it = output_set_.begin(); it != output_set_.end(); it++)
//	{
//		if (!output_set_[1].polar_)
//		{
//			if (bSingleType)
//			{
//				val = ((~val) & 0x01);
//			}
//			else
//			{
//				val = ((~val) & 0x03);
//			}
//		}
//		int ret = da_op_.SaveOriYxVal(output_set_[1].index_,val,true/*terminalPtr->getInitCommPointFlag()*/);
//		if (ret == 5)
//		{
//			count++;
//		}
//		if (count > 0)
//		{
//			da_op_.TrigCosEvent(output_set_[1].index_,val,bSingleType);
//		}
//
//	}
//
//	return true;
//}
//
//void CAlgorithmDA::SignReset()
//{
//	ResetOutputYx(0,true);
//
//	for(vector<share_switchnode_ptr>::iterator it = OriSwitchNodes_.begin(); it != OriSwitchNodes_.end(); it++)
//	{
//		(*it)->ResetOutputYx(0,true);
//	}
//}
//
//void CAlgorithmDA::GetLoad()
//{
//    size_t value1=da_op_.getYcVal(YcOverLoadI_);
//	size_t value2=da_op_.getYcVal(YcOverLoadII_);
//	if((value1<=5000)&&((value1>=100)))
//	{
//       YcOverLoadValue1_=value1;
//	}
//	if((value2<=5000)&&((value2>=100)))
//	{
//		YcOverLoadValue2_=value2;
//	}
//	for(vector<share_switchnode_ptr>::iterator it = OriSwitchNodes_.begin(); it != OriSwitchNodes_.end(); it++)
//	{
//		(*it)->GetLoad();
//	}
//	
//}
//
//void CAlgorithmDA::handle_timerSignReset(const boost::system::error_code& error)
//{
//	if(!error)
//	{
//		SignReset();
//	}
//
//}
//void CAlgorithmDA::handle_timerGetLoad(const boost::system::error_code& error)
//{
//	if(!error)
//	{
//		GetLoad();
//		ResetTimerGetLoad(true,5);
//	}
//
//}
//
//void CAlgorithmDA::ResetTimerSignReset(bool bContinue,unsigned short timeVal)
//{
//	if (bContinue)
//	{
//		TimerSignReset_.expires_from_now(boost::posix_time::seconds(timeVal));
//		TimerSignReset_.async_wait(boost::bind(&CAlgorithmDA::handle_timerSignReset,this,boost::asio::placeholders::error));
//	}
//	else
//	{
//		TimerSignReset_.cancel();
//	}
//}
//
//void CAlgorithmDA::ResetTimerGetLoad(bool bContinue,unsigned short timeVal)
//{
//	if (bContinue)
//	{
//		TimerGetLoad_.expires_from_now(boost::posix_time::seconds(timeVal));
//		TimerGetLoad_.async_wait(boost::bind(&CAlgorithmDA::handle_timerGetLoad,this,boost::asio::placeholders::error));
//	}
//	else
//	{
//		TimerGetLoad_.cancel();
//	}
//}
//
//int CAlgorithmDA::CheckLoadOver()
//{
//	//std::cout<<"load"<<YcOverLoadValue1_<<","<<GetSwitchLoad()<<","<<YcOverLoadValue2_<<","<<OverLoadValue_<<","<<std::endl;
//    
//	int value ;
////		if(YcOverLoadValue1_ <= GetSwitchLoad()) value =YcOverLoadValue2_;
////		else value=(YcOverLoadValue1_-GetSwitchLoad())+YcOverLoadValue2_;
//	value=(GetSwitchLoad())+YcOverLoadValue2_;
//	std::cout<<"value"<<value<<std::endl;
//	if(value >= OverLoadValue_)
//	{
//		return 1;
//	}
//	
//	return 0;
//}
//
//int CAlgorithmDA::GetSwitchLoad()
//{
////	vector<share_switchnode_ptr>::iterator it = ykOpenSet_.end(); //隔离故障
//		return ykOpenSet_[1]->GetSwitchLoad();//(*it)
//}
//
//bool CAlgorithmDA::CheckFASwitchDisable()
//{
////	std::vector<stInOut>::iterator it = input_set_.begin();
//	{
//		size_t yxindex = CheckFADisable_;
//		//bool bPolar = input_set_[0].polar_;
//
//		bool bSingleYx = (da_op_.getYxType(yxindex) == DataBase::single_yx_point);
//		typeYxval yxval = da_op_.getYxVal(yxindex);
//
//
//		bool bRet = false;
//			/*if (bPolar)*/
//			{
//	//			bRet = (yxval == 0x00);
//			}
//			//else
//			{
//				bRet = (yxval == 0x01);
//			}
//
//		if (!bRet)
//		{
//			std::cout<<"硬压板或FA功能开关退出"<<std::endl;
//			return true;
//		}
//	}
//	return false;
//}

}; //namespace FeederAutomation 

