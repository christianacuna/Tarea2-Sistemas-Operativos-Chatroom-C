#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

/**
 * Opens the file if available and can be read
 *
 * @param filename name of thefile to open
 * @return a File type reference
 */
FILE *openFile(char *filename);

/**
 * Writes to a new file
 *
 * @param filename
 * @param content
 */
void writeFile(char *filename, char *content);

#endif // FILE_MANAGER_H