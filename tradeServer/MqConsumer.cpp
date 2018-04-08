#include "StdAfx.h"
//#include "trade.h"
#include "MqConsumer.h"
#include "gobal.h"
//extern TradeSystem* sys;

//extern TradeSystem* GetSystem();

//处理总请求
void DealTotalReq(string& text, int funid);

//extern TradeSystem* sys;//交易系统


MqConsumer::MqConsumer(const std::string& brokerURI, const std::string& strDest, bool useTopic, bool sessionTransacted)
{
	connection = NULL;// (NULL),
	session = NULL;
	destination = NULL;
	consumer = NULL;
	m_useTopic = useTopic;
	m_sessionTransacted = sessionTransacted;
	m_brokerURI = brokerURI;
	m_strDestName = strDest;
	_funid = 0;
}


MqConsumer::~MqConsumer()
{
	cleanup();
}

void MqConsumer::setfunc(int fun)
{
	_funid = fun;
}
int MqConsumer::getfun()
{
	return _funid;

}

void MqConsumer::close() {
	this->cleanup();
}

void MqConsumer::run() 
{

	try {

		// Create a ConnectionFactory
		auto_ptr<ConnectionFactory> connectionFactory(
			ConnectionFactory::createCMSConnectionFactory(m_brokerURI));

		// Create a Connection
		connection = connectionFactory->createConnection(pInfo->_mqName.c_str(),pInfo->_mqPwd.c_str());
		connection->start();
		connection->setExceptionListener(this);

		// Create a Session
		if (this->m_sessionTransacted == true) {
			session = connection->createSession(Session::SESSION_TRANSACTED);
		}
		else {
			session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
		}

		// Create the destination (Topic or Queue)
		if (m_useTopic) {
			destination = session->createTopic(m_strDestName);
		} else {
			destination = session->createQueue(m_strDestName);
		}

		// Create a MessageConsumer from the Session to the Topic or Queue
		consumer = session->createConsumer(destination);

		consumer->setMessageListener(this);

		std::cout.flush();
		std::cerr.flush();

		// Indicate we are ready for messages.
		//latch.countDown();

		// Wait while asynchronous messages come in.
		//doneLatch.await(waitMillis);
		getchar();

	}
	catch (CMSException& e) {
		// Indicate we are ready for messages.
		//latch.countDown();
		e.printStackTrace();
	}
}

// Called from the consumer since this class is a registered MessageListener.
void MqConsumer::onMessage(const Message* message) 
{

	static int count = 0;

	try {
		count++;
		const TextMessage* textMessage = dynamic_cast<const TextMessage*> (message);
		string text = "";

		if (textMessage != NULL) {
			text = textMessage->getText();
		}
		else {
			text = "NOT A TEXTMESSAGE!";
		}

		printf("Message #%d Received: %s, dest:%s, funid=%d\n", count, text.c_str(), m_strDestName.c_str(), _funid);
		Sleep(1);
		//if (strcmp(m_strDestName.c_str(), "DeleteToTrade") == 0)
		//{//撤单
		//	_funid = eFunctionID_OrderDelete;
		//}
		DealTotalReq(text, _funid);

	}
	catch (CMSException& e) {
		e.printStackTrace();
	}

	// Commit all messages.
	if (this->m_sessionTransacted) {
		session->commit();
	}

	// No matter what, tag the count down latch until done.
	//doneLatch.countDown();
}

// If something bad happens you see it here as this class is also been
// registered as an ExceptionListener with the connection.
void MqConsumer::onException(const CMSException& ex AMQCPP_UNUSED) {
	printf("CMS Exception occurred.  Shutting down client.\n");
	ex.printStackTrace();
	//exit(1);
}

void MqConsumer::cleanup() {
	if (connection != NULL) {
		try {
			connection->close();
		}
		catch (cms::CMSException& ex) {
			ex.printStackTrace();
		}
	}

	// Destroy resources.
	try {
		delete destination;
		destination = NULL;
		delete consumer;
		consumer = NULL;
		delete session;
		session = NULL;
		delete connection;
		connection = NULL;
	}
	catch (CMSException& e) {
		e.printStackTrace();
	}
}
