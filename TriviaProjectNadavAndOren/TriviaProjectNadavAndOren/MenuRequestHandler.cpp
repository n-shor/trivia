#include "MenuRequestHandler.h"
#include "RequestHandlerFactory.h"
#include "RoomAdminRequestHandler.h"

MenuRequestHandler::MenuRequestHandler(std::string username) : m_user(username)
{
}

RequestResult MenuRequestHandler::handleRequest(const RequestInfo& requestInfo)
{
	switch (requestInfo.messageCode)
	{
	case CreateRoom:
		return createRoom(requestInfo);
	case GetRooms:
		return getRooms(requestInfo);
	case GetPlayersInRoom:
		return getPlayersInRoom(requestInfo);
	case JoinRoom:
		return joinRoom(requestInfo);
	case GetStatistics:
		return getPersonalStats(requestInfo);
	case Logout:
		RequestHandlerFactory::getInstance().getLoginManager().logout(m_user.getUsername());
		return signout(requestInfo);
	case GetHighScore:
		return getHighScore(requestInfo);
	case AddQuestion:
		return addQuestion(requestInfo);
	}

	RequestResult r;
	r.username = m_user.getUsername();
	ErrorResponse e;
	e.message = "irrelevant message";
	r.response = JsonResponsePacketSerializer::serializeResponse(e);
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	return r;
}

bool MenuRequestHandler::isRequestRelevant(const RequestInfo& requestInfo) const
{
	return requestInfo.messageCode == CreateRoom || requestInfo.messageCode == GetRooms || requestInfo.messageCode == GetPlayersInRoom ||
		requestInfo.messageCode == JoinRoom || requestInfo.messageCode == GetStatistics || requestInfo.messageCode == Logout || requestInfo.messageCode == GetHighScore || requestInfo.messageCode == AddQuestion;
}

RequestResult MenuRequestHandler::signout(const RequestInfo)
{
	RequestResult r;
	r.username = m_user.getUsername();
	LogoutResponse lr;
	lr.status = signedOut;
	r.newHandler = RequestHandlerFactory::getInstance().createLoginRequestHandler();
	r.response = JsonResponsePacketSerializer::serializeResponse(lr);
	return r;
}

RequestResult MenuRequestHandler::getRooms(const RequestInfo)
{
	RequestResult r;
	r.username = m_user.getUsername();
	GetRoomsResponse grr;
	grr.rooms = RequestHandlerFactory::getInstance().getRoomManager().getRooms();
	grr.status = GetRoomsSuccessful;
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::getPlayersInRoom(const RequestInfo ri)
{
	GetPlayersInRoomRequest gpir = JsonRequestPacketDeserializer::deserializeGetPlayersRequest(ri);
	RequestResult r;
	r.username = m_user.getUsername();
	GetPlayesInRoomResponse grr;
	grr.players = RequestHandlerFactory::getInstance().getRoomManager().getRoom(gpir.roomId).getAllUsers();
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::getPersonalStats(const RequestInfo)
{
	RequestResult r;
	r.username = m_user.getUsername();
	getPersonalStatsResponse grr;
	grr.statistics = RequestHandlerFactory::getInstance().getStatisticsManager().getUserStatistics(m_user.getUsername());
	grr.status = GetPersonalStatsSuccessful;
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::getHighScore(const RequestInfo)
{
	RequestResult r;
	r.username = m_user.getUsername();
	getHighScoreResponse grr;
	grr.statistics = RequestHandlerFactory::getInstance().getStatisticsManager().getHighScore();
	grr.status = GetHighScoreSuccessful;
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::joinRoom(const RequestInfo ri)
{
	JoinRoomRequest gpir = JsonRequestPacketDeserializer::deserializeJoinRoomRequest(ri);
	RequestResult r;
	r.username = m_user.getUsername();
	JoinRoomResponse grr;
	if (RequestHandlerFactory::getInstance().getRoomManager().getRoomState(gpir.roomId) == isActive ||
		RequestHandlerFactory::getInstance().getRoomManager().getRoom(gpir.roomId).getRoomData().currentPlayers == 
		RequestHandlerFactory::getInstance().getRoomManager().getRoom(gpir.roomId).getRoomData().maxPlayers) //checking if the room is active or full
	{
		r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
		grr.status = joinRoomUnSuccessful;
		r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	}

	else
	{
		std::lock_guard<std::mutex> lock(RequestHandlerFactory::getInstance().getRoomManager().m_roomsMutex);

		RequestHandlerFactory::getInstance().getRoomManager().getRoom(gpir.roomId).addUser(m_user);
		grr.status = joinRoomSuccessful;
		r.newHandler = RequestHandlerFactory::getInstance().createRoomMemberRequestHandler(m_user, RequestHandlerFactory::getInstance().getRoomManager().getRoom(gpir.roomId));
		r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	}

	return r;
}

RequestResult MenuRequestHandler::createRoom(const RequestInfo ri)
{
	CreateRoomRequest gpir = JsonRequestPacketDeserializer::deserializeCreateRoomRequest(ri);
	RequestResult r;
	r.username = m_user.getUsername();
	CreateRoomResponse grr;
	if (gpir.questionCount > RequestHandlerFactory::getInstance().getStatisticsManager().getDB()->getQuestionCount())
	{
		RoomData rd;
		rd.isActive = 0;
		rd.currentPlayers = 0; //we will add the creator later and then it'll be 1
		rd.maxPlayers = 0;
		rd.name = "";
		rd.adminName = "";
		rd.numOfQuestionsInGame = 0;
		rd.timePerQuestion = 0;
		rd.id = 0;

		grr.status = theServerDoesntHaveEnoughQuestions;
		grr.roomId = 0;
		grr.adminName = "";
		r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
		r.response = JsonResponsePacketSerializer::serializeResponse(grr);
		return r;
	}
	RoomData rd;
	rd.isActive = isntActive;
	rd.currentPlayers = 0; //we will add the creator later and then it'll be 1
	rd.maxPlayers = gpir.maxUsers;
	rd.name = gpir.roomName;
	rd.adminName = m_user.getUsername();
	rd.numOfQuestionsInGame = gpir.questionCount;
	rd.timePerQuestion = gpir.answerTimeout;
	RequestHandlerFactory::getInstance().getRoomManager().currId++;
	rd.id = RequestHandlerFactory::getInstance().getRoomManager().currId;

	RequestHandlerFactory::getInstance().getRoomManager().createRoom(rd, m_user);
	grr.status = CreateRoomSuccessful;
	grr.roomId = rd.id;
	grr.adminName = m_user.getUsername();
	r.newHandler = RequestHandlerFactory::getInstance().createRoomAdminRequestHandler(m_user, RequestHandlerFactory::getInstance().getRoomManager().getRoom(rd.id));
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::addQuestion(const RequestInfo ri)
{
	AddQuestionRequest adr = JsonRequestPacketDeserializer::deserializeAddQuestionRequest(ri);
	RequestResult r;
	r.username = m_user.getUsername();
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());

	bool retStat = RequestHandlerFactory::getInstance().getStatisticsManager().getDB()->addQuestion(RequestHandlerFactory::getInstance().getStatisticsManager().getDB()->getQuestionCount()+1, adr.question, adr.optionA, adr.optionB, adr.optionC, adr.optionD, adr.correctAnswer);
	AddQuestionResponse aqr;
	if (retStat)
	{
		aqr.status = QuestionAdded;
	}
	else {
		aqr.status = InvalidQuestion;
	}

	r.response = JsonResponsePacketSerializer::serializeResponse(aqr);
	return r;
}
