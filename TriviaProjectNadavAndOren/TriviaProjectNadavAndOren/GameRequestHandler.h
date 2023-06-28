#pragma once
#include "MenuRequestHandler.h"
#include "RequestHandlerFactory.h"
#include "JsonResponsePacketSerializer.hpp"
#include "JsonRequestPacketDeserializer.hpp"
#include "RoomMemberRequestHandler.h"

enum GameRequestTypes { getQuestionReq = 4, submitAnswerReq, getGameResultsReq, leaveGameReq, LeaderBoard = 240, checkForEnd = 255};
enum GameRequeststatus { getQuestionSuccessful = 8, gameEnded, leaveGameResponse};

class GameRequestHandler : public IRequestHandler
{
private:
	Game& m_game;
	LoggedUser m_user;
	RequestResult getQuestion(const RequestInfo);
	RequestResult submitAnswer(const RequestInfo);
	RequestResult getGameResults(const RequestInfo);
	RequestResult leaveGame(const RequestInfo);
	RequestResult leaderboard(const RequestInfo);
	RequestResult checkGameEnd(const RequestInfo);
	bool isRequestRelevant(const RequestInfo& requestInfo) override;
	static bool m_gameEnded;

public:
	GameRequestHandler(std::string, Game&);
	RequestResult handleRequest(const RequestInfo& requestInfo) override;
};