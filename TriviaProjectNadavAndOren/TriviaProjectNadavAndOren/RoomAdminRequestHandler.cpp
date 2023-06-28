#include "RoomAdminRequestHandler.h"
#include "RequestHandlerFactory.h"
#include "GameRequestHandler.h"

bool RoomAdminRequestHandler::isRequestRelevant(const RequestInfo& requestInfo)
{
	return requestInfo.messageCode == CloseRoom || requestInfo.messageCode == StartGame || requestInfo.messageCode == GetRoomsState;
}

RequestResult RoomAdminRequestHandler::closeRoom(RequestInfo)
{
	RequestHandlerFactory::getInstance().getRoomManager().deleteRoom(m_room.getRoomData().id);

	RequestResult r;
	CloseRoomResponse crr;
	crr.status = closeRoomSuccessful;
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	r.response = JsonResponsePacketSerializer::serializeResponse(crr);
	r.username = m_user.getUsername();
	return r;
}

RequestResult RoomAdminRequestHandler::startGame(RequestInfo)
{
	RequestResult r;
	StartGameResponse sgr;


	try {
		sgr.status = startGameSuccessful;
		m_room.activateRoom();
		r.newHandler = RequestHandlerFactory::getInstance().createGameRequestHandler(m_user,
			RequestHandlerFactory::getInstance().getGameManager().getGames()[RequestHandlerFactory::getInstance().getGameManager().createGame(m_room)]);
		r.response = JsonResponsePacketSerializer::serializeResponse(sgr);
		return r;
	}
	catch (std::logic_error e)
	{
		if (e.what() == notEnoughQuestionsMessage)
		{
			sgr.status = theServerDoesntHaveEnoughQuestions;
		}
		else {
			sgr.status = startGameUnsuccessful;
		}
		m_room.deactivateRoom();
		r.newHandler = RequestHandlerFactory::getInstance().createRoomAdminRequestHandler(m_user, m_room);
		r.response = JsonResponsePacketSerializer::serializeResponse(sgr);
		r.username = m_user.getUsername();
		return r;
	}
}

template <typename T>
bool contains(std::vector<T> vec, const T& elem)
{
	bool result = false;
	if (find(vec.begin(), vec.end(), elem) != vec.end())
	{
		result = true;
	}
	return result;
}

RequestResult RoomAdminRequestHandler::getRoomsState(RequestInfo)
{
	try {
		if (!contains(m_room.getAllUsers(), m_room.getRoomData().adminName))
		{
			throw std::logic_error("the user is not in the room");
		}
		RequestResult r;
		GetRoomStateResponse grsr;
		grsr.answerTimeout = m_room.getRoomData().timePerQuestion;
		grsr.hasGameBegun = m_room.getRoomData().isActive;
		grsr.questionCount = m_room.getRoomData().numOfQuestionsInGame;
		grsr.players = m_room.getAllUsers();
		grsr.status = getRoomsStateRes;
		r.response = JsonResponsePacketSerializer::serializeResponse(grsr);
		r.newHandler = RequestHandlerFactory::getInstance().createRoomAdminRequestHandler(m_user, m_room);
		r.username = m_user.getUsername();
		return r;
	}
	catch (std::logic_error le)
	{
		std::cout << le.what() << "\n";
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
	}
	catch(...)
	{

	}
	RequestResult r;
	ErrorResponse e;
	e.message = "room closed";
	r.response = JsonResponsePacketSerializer::serializeResponse(e);
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	r.username = m_user.getUsername();
	return r;
}

RoomAdminRequestHandler::RoomAdminRequestHandler(std::string username, Room& room) : m_room(room), m_user(username)
{
}

RequestResult RoomAdminRequestHandler::handleRequest(const RequestInfo& requestInfo)
{
	if (isRequestRelevant(requestInfo))
	{
		switch (requestInfo.messageCode)
		{
		case CloseRoom:
			return this->closeRoom(requestInfo);
		case StartGame:
			return this->startGame(requestInfo);
		case GetRoomsState:
			return this->getRoomsState(requestInfo);
		}
	}
	RequestResult r;
	ErrorResponse e;
	e.message = "irrelevant message";
	r.response = JsonResponsePacketSerializer::serializeResponse(e);
	r.newHandler = RequestHandlerFactory::getInstance().createMenuRequestHandler(m_user.getUsername());
	r.username = m_user.getUsername();
	return r;
}

