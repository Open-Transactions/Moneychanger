#include "FileHandler.h"

/*
 *remove a file
 */
bool FileHandler::removeFile(QString fileName)
{
    bool ret = false;

   /*
    *remove  file
    */
   ret = QFile::remove(fileName);

   if(ret == false)
   {
       /*
        *Gui message that informs for a possible error
        */
       QMessageBox::critical(0,fileHandlerstr,fileNotRemovedStr);

       /*
        *@return false
        */
       return ret;
   }
  /*
   *Gui message that informs for a succesfull removal
   */
   QMessageBox::information(0,fileHandlerstr,fileRemovedStr);

   /*
    *@return true
    */
   return ret;

}
/*
 *check if a file exist
 */
bool FileHandler::isFileExist(QString fileName)
{
    bool ret= false;

    /*
     *check if the file "fileName" exist
     */
    ret = QFile::exists(fileName);

    if(ret == false)
    {
        /*
         *Gui message that informs that the file was not found
         */
        QMessageBox::critical(0,fileHandlerstr,fileNotExistStr);

        /*
         *@return false
         */
        return ret;

    }
    /*
     *return true
     */
    return ret;
}
