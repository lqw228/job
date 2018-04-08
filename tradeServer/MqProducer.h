#pragma once
#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>
#include <decaf/lang/System.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>



using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;
using namespace std;

class MqProducer
{
public:
	MqProducer();
	~MqProducer();

	bool CreateProduct(std::string& brokerURI, string& strDest, bool bTopic = false);
	void SendData(string& text);
private:
	void cleanup();
private:

	Connection* m_connection;
	Session* m_session;
	Destination* m_destination;
	MessageProducer* m_producer;
	bool m_useTopic;
	bool m_sessionTransacted;
	std::string m_brokerURI;
	string m_strDest;
	//std::recursive_mutex m_mtxFunction;
};

