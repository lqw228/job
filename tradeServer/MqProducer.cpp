#include "StdAfx.h"
#include "MqProducer.h"


MqProducer::MqProducer()
{
	m_connection  = NULL;
	m_session     = NULL;
	m_destination = NULL;
	m_producer    = NULL;
	m_useTopic    = false;
	m_sessionTransacted = false;
}


MqProducer::~MqProducer()
{
	cleanup();
}

void MqProducer::cleanup() 
{

	if (m_connection != NULL) {
		try {
			m_connection->close();
		}
		catch (cms::CMSException& ex) {
			ex.printStackTrace();
		}
	}

	// Destroy resources.
	try {
		delete m_destination;
		m_destination = NULL;
		delete m_producer;
		m_producer = NULL;
		delete m_session;
		m_session = NULL;
		delete m_connection;
		m_connection = NULL;
	}
	catch (CMSException& e) {
		e.printStackTrace();
	}
}

void MqProducer::SendData(string& text)
{
	if (m_session)
	{		
		std::auto_ptr<TextMessage> message(m_session->createTextMessage(text));
		m_producer->send(message.get());
	}
	else
	{
		printf("SendData, session=null, %s\n", text.c_str());
	}
}

bool MqProducer::CreateProduct(std::string& brokerURI, string& strDest, bool bTopic)
{
	bool bret = true;
	try {

		// Create a ConnectionFactory
		auto_ptr<ConnectionFactory> connectionFactory(ConnectionFactory::createCMSConnectionFactory(brokerURI));

		;
		// Create a Connection
		m_connection = connectionFactory->createConnection(pInfo->_mqName.c_str(),pInfo->_mqPwd.c_str());//"gooking","activemq14:28"
		if (m_connection == NULL)
		{
			return false;
		}
		//ActiveMQConnection::setPassword(
		m_connection->start();

		// Create a Session
		m_session = m_connection->createSession(Session::AUTO_ACKNOWLEDGE);
		if (NULL == m_session)
		{
			printf("session ==  NULL, ERROR\n");
			bret = false;
			return bret;
		}
		m_strDest = strDest;
		// Create the destination (Topic or Queue)
		if (bTopic)
		{
			m_destination = m_session->createTopic(strDest);
		}
		else{
			m_destination = m_session->createQueue(strDest);
		}

		// Create a MessageProducer from the Session to the Topic or Queue
		m_producer = m_session->createProducer(m_destination);
		if (m_producer == NULL)
		{
			return false;
		}
		m_producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

	}
	catch (CMSException& e) {
		e.printStackTrace();
		return false;
	}

	return true;

}
