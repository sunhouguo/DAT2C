#pragma once

#ifndef MYBTREE_H
#define MYBTREE_H

#include <iostream>

/**********************************************************************************************************
用来表示森林的二叉树类，左子女，右兄弟
**********************************************************************************************************/
namespace CentralizedFA {

template <class ElemType> class CMyBTree;

template <class ElemType> class CMyBTreeNode
{
	friend class CMyBTree<ElemType>;
private:
	ElemType data;
	CMyBTreeNode<ElemType> *firstChild, *nextSibling, *link; 			//左孩子，右兄弟，衍生枝连接的结点
	CMyBTreeNode(ElemType value=0, CMyBTreeNode<ElemType> *fc=NULL, CMyBTreeNode<ElemType> *ns=NULL, CMyBTreeNode<ElemType> *lk=NULL)
		:data(value), firstChild(fc), nextSibling(ns), link(lk){}
};

template <class ElemType> class CMyBTree
{
public:
	CMyBTree()
	{
		root=current=NULL;
	}

	bool Root();           																				//current指向树根
	bool FirstChild();     																				//current指向左孩子
	bool NextSibling();    																				//current指向右兄弟
	void BuildRoot(ElemType rootVal);  																		//建立树根
	void InsertChild(ElemType value);  																		//插入子女结点
	bool Find(ElemType target);        																		//根据目标值找结点
	bool Parent();                 																		//current指向父结点
	bool IsRoot();                 																		//判断当前结点是否位根结点
	void RemovesubTree();          																		//删除以当前结点为根的子树
	void SetLink(CMyBTreeNode<ElemType> * SrcLink,CMyBTreeNode<ElemType> * DesLink);  //设置衍生枝
	ElemType GetCurrentData();         																		//获得当前结点的值
	ElemType GetLinkData();            																		//获得当前结点衍生枝连接结点的值
	CMyBTreeNode<ElemType> * GetCurrentPointer();  															//获得指向当前结点的指针
	void SetCurrentPointer(CMyBTreeNode<ElemType> * target);  												//将current结点指向目标结点
	bool IsEmpty()                  																	//判断当前结点是否位为空
	{
		return current==NULL;
	}
	bool IsLeaf()                   																	//判断当前结点是否为树叶
	{
		return current->firstChild==NULL;
	}
	bool HasLinkNode()              																	//判断当前结点是否有衍生枝
	{
		return current->link!=NULL;
	}

private:
	CMyBTreeNode<ElemType> *root, *current;
	bool Find( CMyBTreeNode<ElemType> *p, ElemType target);
	void RemovesubTree( CMyBTreeNode<ElemType> *p);
	bool FindParent( CMyBTreeNode<ElemType> *t, CMyBTreeNode<ElemType> *p);
};

//建立树根
template <class ElemType> void CMyBTree<ElemType>::BuildRoot(ElemType rootVal)
{
	root=current=new CMyBTreeNode<ElemType>(rootVal);
}

//判断当前结点是否位根结点
template <class ElemType> bool CMyBTree<ElemType>::IsRoot()
{
	if(root==NULL)
		return false;
	else
		return current==root;
}

//current指向树根
template <class ElemType> bool CMyBTree<ElemType>::Root()                        
{
	if(root==NULL)
	{
		current = NULL;
		return false;
	}
	else
	{
		current=root;
		return true;
	}
}

//设置衍生枝
template <class ElemType> void CMyBTree<ElemType>::SetLink(CMyBTreeNode<ElemType> * SrcLink,CMyBTreeNode<ElemType> * DesLink)
{
	SrcLink->link=DesLink;
	DesLink->link=SrcLink;
	//printf("SrcLink=%d\n",SrcLink->data);
	//printf("DesLink=%d\n",DesLink->data);
}

//获得当前结点的值
template <class ElemType>	ElemType CMyBTree<ElemType>::GetCurrentData()
{
	if(!IsEmpty())
	{
		return current->data;
	}

	return ElemType();
}

//获得当前结点衍生枝连接结点的值
template <class ElemType> ElemType CMyBTree<ElemType>::GetLinkData()
{
	if(HasLinkNode())
	{
		return current->link->data;
	}

	return ElemType();
}

//获得指向当前结点的指针
template <class ElemType>	CMyBTreeNode<ElemType> * CMyBTree<ElemType>::GetCurrentPointer()
{
	return current;
}

//将current结点指向目标结点
template <class ElemType>	void CMyBTree<ElemType>::SetCurrentPointer(CMyBTreeNode<ElemType> * target)
{
	current=target;
}

//current指向左孩子
template <class ElemType> bool CMyBTree<ElemType>::FirstChild()
{
	if(current!=NULL&&current->firstChild!=NULL)
	{
		current=current->firstChild;
		return true;
	}
	current=NULL;
	return false;
}

//current指向右兄弟
template <class ElemType> bool CMyBTree<ElemType>::NextSibling()
{
	if(current!=NULL&&current->nextSibling!=NULL)
	{
		current = current -> nextSibling;
		return true;
	}
	current=NULL;
	return false;
}

//插入子女结点
template <class ElemType> void CMyBTree<ElemType>::InsertChild(ElemType value)
{
	CMyBTreeNode<ElemType> * newNode =new CMyBTreeNode<ElemType>(value);
	newNode->data=value;
	if(current->firstChild==NULL)
	{
		current->firstChild=newNode;
		//printf("Current firstChild data=%d\n",current->firstChild->data);
	}
	else
	{
		CMyBTreeNode<ElemType> *p =current->firstChild;
		while(p->nextSibling != NULL)
		{
			p=p->nextSibling;
		}
		p->nextSibling=newNode;
		//printf("firstChild nextSibling data=%d\n",newNode->data);
	}

}

//根据目标值找结点
template <class ElemType>	bool CMyBTree<ElemType>::Find( CMyBTreeNode<ElemType> *p, ElemType target)
{
	bool result=false;
	if(p->data==target)
	{
		result=true;
		current=p;
		//printf("Find success=%d\n",current->data);
	}
	else
	{
		CMyBTreeNode<ElemType> *q = p->firstChild;
		while(q!=NULL&&!(result = Find(q,target)))
		{
			q = q->nextSibling;
			//printf("Finding\n");
		}
	}
	return result;
}

//根据目标值找结点
template <class ElemType> bool CMyBTree<ElemType>::Find(ElemType target)
{
	if(root==NULL)
	{
		return false;
	}
	return Find(root,target);
}

//Parent函数调用的内部函数
template <class ElemType> bool CMyBTree<ElemType>::FindParent( CMyBTreeNode<ElemType> *t, CMyBTreeNode<ElemType> *p)
{
	CMyBTreeNode<ElemType> *q = t->firstChild;
	while(q!=NULL&&q!=p)
	{
		bool ret=FindParent(q,p);
		if(ret)
		{
			return ret;
		}
		q = q->nextSibling;
	}
	if(q!=NULL && q==p)
	{
		current=t;
		return true;
	}
	else
		return false;
}

//current指向父结点
template <class ElemType> bool CMyBTree<ElemType>::Parent()
{
	CMyBTreeNode<ElemType> * p =current, *t;
	if(current == NULL||current == root)
	{
		current = NULL;
		return false;
	}
	t = root;
	bool ret = FindParent(t,p);
	return ret;
}

//删除以当前结点为根的子树
template <class ElemType> void CMyBTree<ElemType>::RemovesubTree( CMyBTreeNode<ElemType> *p)
{
	CMyBTreeNode<ElemType> * q=p->firstChild, *next;
	while(q!=NULL)
	{
		next=q->nextSibling;
		RemovesubTree(q);
		q=next;
	}
	delete p;
}

//删除以当前结点为根的子树
template <class ElemType> void CMyBTree<ElemType>::RemovesubTree()
{
	if(current!=NULL)
	{
		if(current==root)
		{
			root=NULL;
		}
		RemovesubTree(current);
		current=NULL;
	}
}

}; //namespace FeederAutomation

#endif //#ifndef MYBTREE_H

