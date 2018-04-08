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

class MqConsumer : public ExceptionListener,
	public MessageListener,
	public Runnable
{
public:
	MqConsumer(const std::string& brokerURI, const std::string& strDest, bool useTopic = false, bool sessionTransacted = false);
	~MqConsumer();
public:
	void setfunc(int fun);
	int getfun();

private:

	//CountDownLatch latch;
	//CountDownLatch doneLatch;
	Connection* connection;
	Session* session;
	Destination* destination;
	MessageConsumer* consumer;
	//long waitMillis;
	bool m_useTopic;
	bool m_sessionTransacted;
	std::string m_brokerURI;
	std::string m_strDestName;
	int _funid;

private:
	MqConsumer(const MqConsumer&);
	MqConsumer& operator=(const MqConsumer&);

private:
	void cleanup();

public:
	virtual void run();
	// Called from the consumer since this class is a registered MessageListener.
	virtual void onMessage(const Message* message);
	// If something bad happens you see it here as this class is also been
	// registered as an ExceptionListener with the connection.
	virtual void onException(const CMSException& ex AMQCPP_UNUSED);

void close();
};

