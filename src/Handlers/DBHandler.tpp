// Template implementation for DBHandler.h.

/**
 * Run a query and for each returned record, execute a callback.  The
 * callback is passed the QSqlRecord for each result.
 * @param run The query to run.
 * @param cb The callback function called.
 * @return True in case of success.
 */
template<typename T>
  bool
  DBHandler::queryMultiple (QString run, T cb)
{
  QMutexLocker locker(&dbMutex);
  QSqlQuery query(db);

  if (!db.isOpen ())
    return false;

  const bool ok = query.exec (run);
  if (!ok)
    {
      qDebug () << "runQuery: QSqlQuery::lastError: "
                << query.lastError ().text ();
      qDebug () << QString("THE QUERY (that caused the error): %1").arg (run);
      return false;
    }

  while (query.next ())
    cb (query.record ());

  return true;
}
