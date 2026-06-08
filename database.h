#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>

/**
 * @brief SQLite request journal shared by the TCP server.
 *
 * The class is a singleton: copying and creating additional instances are
 * forbidden. By default, the database is created in the server working
 * directory so it is easy to inspect during a demonstration.
 */
class Database
{
public:
    /**
     * @brief Returns the single database object.
     * @return Reference to the server database singleton.
     */
    static Database &instance();

    /**
     * @brief Opens SQLite and creates or upgrades the requests table.
     * @return true when the database is ready for queries.
     */
    bool open();

    /**
     * @brief Stores one processed client request.
     * @param request Raw request received from a client.
     * @param response Response returned to the client.
     * @return true when the row was inserted.
     */
    bool saveRequest(const QString &request, const QString &response);

    /**
     * @brief Formats the request journal for the show_db command.
     * @return Human-readable database rows or an error message.
     */
    QString getAllRequests() const;

    /**
     * @brief Returns the configured SQLite file name or special SQLite URI.
     * @return Database path passed to the SQLite driver.
     */
    QString databasePath() const;

    /**
     * @brief Reports whether SQLite is currently open.
     * @return true when the connection can be used.
     */
    bool isOpen() const;

private:
    Database();
    ~Database();
    Database(const Database &) = delete;
    Database &operator=(const Database &) = delete;

    bool ensureSchema();

    QSqlDatabase mDatabase;
    QString mDisplayDatabasePath;
    QString mLastError;
};

#endif // DATABASE_H
