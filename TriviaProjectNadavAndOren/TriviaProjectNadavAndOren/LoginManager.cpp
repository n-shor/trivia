#include "LoginManager.h"
#include "LoggedUser.h"
#include <mutex>
#include <map>

std::unique_ptr<IDatabase> LoginManager::m_database = nullptr;
std::mutex LoginManager::m_database_mutex;

LoginManager::LoginManager()
{
    std::lock_guard<std::mutex> lock(m_database_mutex);
    if (m_database == nullptr) {
        m_database = std::make_unique<SqliteDatabase>();
        m_database->open();
    }
}

int LoginManager::signup(std::string username, std::string password, std::string email)
{
    std::lock_guard<std::mutex> lock(m_database_mutex);
    if (!m_database->doesUserExist(username)) {
        m_database->addNewUser(username, password, email);
        m_loggedUsers.push_back(LoggedUser(username));
        return SignedUp;
    }
    return UserAlreadyExists;
}

bool containsUser(std::vector<LoggedUser>& LoggedUsers, std::string& user)
{
    for (int i = 0; i < LoggedUsers.size(); i++)
    {
        if (LoggedUsers[i].getUsername() == user)
        {
            return true;
        }
    }
    return false;
}

int LoginManager::login(std::string username, std::string password)
{
    std::lock_guard<std::mutex> lock(m_database_mutex);
    if (m_database->doesUserExist(username) && m_database->doesPasswordMatch(username, password))
    {
        if (containsUser(m_loggedUsers, username))
        {
            return AlreadyLoggedIn;
        }
        m_loggedUsers.push_back(LoggedUser(username));
        return LoggedIn;
    }
    return FailedLogin;
}

int LoginManager::logout(std::string username)
{
    std::lock_guard<std::mutex> lock(m_database_mutex);
    for (auto it = m_loggedUsers.begin(); it != m_loggedUsers.end(); ++it)
    {
        if (it->getUsername() == username)
        {
            m_loggedUsers.erase(it);
            return LoggedOut;
        }
    }
    return NoSuchLoggedUser;
}
