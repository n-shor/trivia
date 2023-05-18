#pragma once

#include "sqlite3.h"
#include "IDatabase.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include "Question.hpp"

class SqliteDatabase : public IDatabase {
private:
    sqlite3* db;

public:
    SqliteDatabase() : db(nullptr) {}

    bool open() override {
        if (sqlite3_open("Trivia.sqlite", &db) != SQLITE_OK) {
            std::cout << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        const char* sqlUsers = "CREATE TABLE IF NOT EXISTS USERS("
            "USERNAME TEXT NOT NULL,"
            "PASSWORD TEXT NOT NULL,"
            "EMAIL TEXT NOT NULL);";

        const char* sqlTrivia = "CREATE TABLE IF NOT EXISTS TriviaQuestions("
            "ID INT PRIMARY KEY,"
            "Question TEXT NOT NULL,"
            "OptionA TEXT NOT NULL,"
            "OptionB TEXT NOT NULL,"
            "OptionC TEXT NOT NULL,"
            "OptionD TEXT NOT NULL,"
            "CorrectAnswer CHAR NOT NULL);";

        const char* sqlStatistics = "CREATE TABLE IF NOT EXISTS STATISTICS("
            "USERNAME TEXT NOT NULL,"
            "TOTAL_GAMES INT DEFAULT 0,"
            "TOTAL_ANSWERS INT DEFAULT 0,"
            "CORRECT_ANSWERS INT DEFAULT 0,"
            "AVERAGE_ANSWER_TIME REAL DEFAULT 0,"
            "SCORE INT DEFAULT 0,"
            "FOREIGN KEY(USERNAME) REFERENCES USERS(USERNAME));";

        if (sqlite3_exec(db, sqlUsers, nullptr, 0, nullptr) != SQLITE_OK ||
            sqlite3_exec(db, sqlTrivia, nullptr, 0, nullptr) != SQLITE_OK ||
            sqlite3_exec(db, sqlStatistics, nullptr, 0, nullptr) != SQLITE_OK)
        {
            std::cout << "Error creating tables: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }

        std::vector<std::string> questions = {
        "INSERT INTO TriviaQuestions VALUES (1, 'John has 8 apples, Susan is late to her train by 23.2 seconds and the air resistance equals 83 newtons. What is the mass of the sun?', '14.5 newtons', '5.972*10^24 kg', '1.989*10^30 kg', 'Louis XV', 'C');",
        "INSERT INTO TriviaQuestions VALUES (2, 'Who let the dogs out?', 'who, who, who, who, who?', 'who, who, who, who, who', 'Joe Mama', 'The Baha men', 'A');",
        "INSERT INTO TriviaQuestions VALUES (3, 'What�s the accurate color of the famous dress?', 'white and gold', 'black and blue', 'blue and gold', 'Israeli flag colors', 'B');",
        "INSERT INTO TriviaQuestions VALUES (4, 'why did the chicken cross the road?', 'BOOM SCHNITZEL!', 'What is America swimsuit?', 'Chaim and Moshe live in a building with 100 floors�', 'to get to the other side', 'D');",
        "INSERT INTO TriviaQuestions VALUES (5, 'If 23 people are in the same room, what�s the chance that 2 people have the same birthday?', '1.2% approximately', '50% approximately', '100% EXACTLY', '0.00534% approximately', 'B');",
        "INSERT INTO TriviaQuestions VALUES (6, 'What is the most streamed song on spotify?', 'Shape of you by ed sheeran', 'Bohemian rhapsody by queen', 'Blinding lights by the weeknd', 'One dance by drake', 'C');",
        "INSERT INTO TriviaQuestions VALUES (7, 'How many US presidents were assassinated?', '4', '0', '2', '3', 'A');",
        "INSERT INTO TriviaQuestions VALUES (8, 'What is the country with the highest Male to Female ratio?', 'UAE', 'Qatar', 'Bahrain', 'Kuwait', 'B');",
        "INSERT INTO TriviaQuestions VALUES (9, 'The richest woman in the world is the _ richest person', '5th', '23rd', '104th', '11th', 'D');",
        "INSERT INTO TriviaQuestions VALUES (10, 'Who is the chess player that was the world champion for the most years?', 'garry kasparov', 'magnus carlsen', 'emanuel lasker', 'Bobby fischer', 'C');"
        };

        for (const std::string& question : questions) {
            if (sqlite3_exec(db, question.c_str(), nullptr, 0, nullptr) != SQLITE_OK) {
                std::cout << "Error inserting question: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                return false;
            }
        }

        std::cout << "SQLite database connection opened and tables created if they didn't exist." << std::endl;
        return true;
    }

    bool close() override {
        sqlite3_close(db);
        std::cout << "SQLite database connection closed." << std::endl;
        return true;
    }

    bool doesUserExist(std::string username) override {
        std::string sql = "SELECT * FROM USERS WHERE USERNAME = '" + username + "';";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        bool userExists = false;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            userExists = true;
        }

        sqlite3_finalize(stmt);
        return userExists;
    }

    bool doesPasswordMatch(std::string username, std::string password) override {
        std::string sql = "SELECT PASSWORD FROM USERS WHERE USERNAME = '" + username + "';";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        bool passwordMatches = false;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string dbPassword = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            passwordMatches = (dbPassword == password);
        }

        sqlite3_finalize(stmt);
        return passwordMatches;
    }

    bool addNewUser(std::string username, std::string password, std::string email) override {
        std::string sql = "INSERT INTO USERS (USERNAME, PASSWORD, EMAIL) VALUES ('" + username + "', '" + password + "', '" + email + "');";

        if (sqlite3_exec(db, sql.c_str(), nullptr, 0, nullptr) != SQLITE_OK) {
            std::cout << "Error inserting user" << std::endl;
            return false;
        }

        std::cout << "User created successfully in the database!" << std::endl;
        return true;
    }

    bool addQuestion(int id, std::string question, std::string optionA, std::string optionB,
        std::string optionC, std::string optionD, char correctAnswer) {
        std::string sql = "INSERT INTO TriviaQuestions (ID, Question, OptionA, OptionB, OptionC, OptionD, CorrectAnswer) "
            "VALUES (" + std::to_string(id) + ", '" + question + "', '" + optionA + "', '" + optionB + "', '" + optionC + "', '" + optionD + "', '" + correctAnswer + "');";

        if (sqlite3_exec(db, sql.c_str(), nullptr, 0, nullptr) != SQLITE_OK) {
            std::cout << "Error inserting question" << std::endl;
            return false;
        }

        std::cout << "Question added successfully to the database!" << std::endl;
        return true;
    }

    Question getQuestion(int id) {
        std::string sql = "SELECT * FROM TriviaQuestions WHERE ID = " + std::to_string(id) + ";";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        Question question;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            question.id = sqlite3_column_int(stmt, 0);
            question.question = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            question.optionA = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            question.optionB = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            question.optionC = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            question.optionD = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            question.correctAnswer = *reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        }

        sqlite3_finalize(stmt);
        return question;
    }

    char getCorrectAnswer(int id) {
        std::string sql = "SELECT CorrectAnswer FROM TriviaQuestions WHERE ID = " + std::to_string(id) + ";";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        char correctAnswer = '\0';
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            correctAnswer = *reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }

        sqlite3_finalize(stmt);
        return correctAnswer;
    }

    float getPlayerAverageAnswerTime(std::string username) {
        std::string sql = "SELECT AVERAGE_ANSWER_TIME FROM STATISTICS WHERE USERNAME = '" + username + "';";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        float avgAnswerTime = 0.0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            avgAnswerTime = sqlite3_column_double(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return avgAnswerTime;
    }

    int getNumOfCorrectAnswers(std::string username) {
        std::string sql = "SELECT CORRECT_ANSWERS FROM STATISTICS WHERE USERNAME = '" + username + "';";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        int correctAnswers = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            correctAnswers = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return correctAnswers;
    }

    int getNumOfTotalAnswers(std::string username) {
        std::string sql = "SELECT TOTAL_ANSWERS FROM STATISTICS WHERE USERNAME = '" + username + "';";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        int totalAnswers = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            totalAnswers = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return totalAnswers;
    }

    int getNumOfPlayerGames(std::string username) {
        std::string sql = "SELECT TOTAL_GAMES FROM STATISTICS WHERE USERNAME = '" + username + "';";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        int totalGames = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            totalGames = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return totalGames;
    }

    int getPlayerScore(std::string username) {
        std::string sql = "SELECT SCORE FROM STATISTICS WHERE USERNAME = '" + username + "';";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Error preparing SQL statement");
        }

        int score = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            score = sqlite3_column_int(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return score;
    }

    std::vector<std::string> getHighScores()
    {
        std::string sql = "SELECT USERNAME, SCORE FROM STATISTICS ORDER BY SCORE DESC LIMIT 5;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Error preparing SQL statement");
        }

        std::vector<std::string> highScoreNames;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::string username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            highScoreNames.push_back(username);
        }

        sqlite3_finalize(stmt);
        return highScoreNames;
    }
};
