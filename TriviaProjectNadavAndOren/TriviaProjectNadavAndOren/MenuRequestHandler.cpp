#include "MenuRequestHandler.h"

MenuRequestHandler::MenuRequestHandler(std::string username, RequestHandlerFactory rhf, RoomManager rm) : m_user(username), m_handlerFactory(rhf)
{
	m_user = LoggedUser(username);
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
		return signout(requestInfo);
	case GetHighScore:
		return getHighScore(requestInfo);
	}

	RequestResult r;
	ErrorResponse e;
	e.message = "irrelevant message";
	r.response = JsonResponsePacketSerializer::serializeResponse(e);
	r.newHandler = m_handlerFactory.createMenuRequestHandler(m_user.getUsername());
	return r;
}

bool MenuRequestHandler::isRequestRelevant(const RequestInfo& requestInfo)
{
	return requestInfo.messageCode >= 0 && requestInfo.messageCode <= 6;
}

RequestResult MenuRequestHandler::signout(RequestInfo)
{
	RequestResult r;
	LogoutResponse lr;
	lr.status = signedOut;
	r.newHandler = m_handlerFactory.createLoginRequestHandler();
	r.response = JsonResponsePacketSerializer::serializeResponse(lr);
	return r;
}

RequestResult MenuRequestHandler::getRooms(RequestInfo)
{
	RequestResult r;
	GetRoomsResponse grr;
	grr.rooms = m_handlerFactory.getRoomManager().getRooms();
	grr.status = GetRoomsSuccessful;
	r.newHandler = m_handlerFactory.createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::getPlayersInRoom(RequestInfo ri)
{
	GetPlayersInRoomRequest gpir = JsonRequestPacketDeserializer::deserializeGetPlayersRequest(ri);
	RequestResult r;
	GetPlayesInRoomResponse grr;
	grr.players = m_handlerFactory.getRoomManager().getRoom(gpir.roomId).getAllUsers();
	r.newHandler = m_handlerFactory.createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::getPersonalStats(RequestInfo)
{
	RequestResult r;
	getPersonalStatsResponse grr;
	grr.statistics = m_handlerFactory.getStatisticsManager().getUserStatistics(m_user.getUsername());
	grr.status = GetPersonalStatsSuccessful;
	r.newHandler = m_handlerFactory.createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::getHighScore(RequestInfo)
{
	RequestResult r;
	getHighScoreResponse grr;
	grr.statistics = m_handlerFactory.getStatisticsManager().getHighScore();
	grr.status = GetHighScoreSuccessful;
	r.newHandler = m_handlerFactory.createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::joinRoom(RequestInfo ri)
{
	JoinRoomRequest gpir = JsonRequestPacketDeserializer::deserializeJoinRoomRequest(ri);
	RequestResult r;
	JoinRoomResponse grr;
	if (m_handlerFactory.getRoomManager().getRoomState(gpir.roomId) == isActive)
	{
		r.newHandler = m_handlerFactory.createMenuRequestHandler(m_user.getUsername());
		grr.status = joinRoomUnSuccessful;
		r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	}
	m_handlerFactory.getRoomManager().getRoom(gpir.roomId).addUser(m_user);
	grr.status = joinRoomSuccessful;
	r.newHandler = m_handlerFactory.createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}

RequestResult MenuRequestHandler::createRoom(RequestInfo ri)
{
	CreateRoomRequest gpir = JsonRequestPacketDeserializer::deserializeCreateRoomRequest(ri);
	RequestResult r;
	CreateRoomResponse grr;

	RoomData rd;
	rd.isActive = isntActive;
	rd.maxPlayers = gpir.maxUsers;
	rd.name = gpir.roomName;
	rd.numOfQuestionsInGame = gpir.questionCount;
	rd.timePerQuestion = gpir.answerTimeout;

	unsigned int max = 0;

	for (int i = 0; i < m_handlerFactory.getRoomManager().getRooms().size(); i++)
	{
		if (m_handlerFactory.getRoomManager().getRooms()[i].id > max)
		{
			max = m_handlerFactory.getRoomManager().getRooms()[i].id;
		}
	}

	rd.id = max + 1;

	m_handlerFactory.getRoomManager().createRoom(m_user, rd);
	grr.status = CreateRoomSuccessful;
	r.newHandler = m_handlerFactory.createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(grr);
	return r;
}
