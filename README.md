# Finder
Build my own Finder application. Can index directories and search through them.

# Commands
- \-add index *directory*;*directory*;*...*.
  - *directory* must have /\* at the end. For example, C:/Program Files (x86)/Steam/\*.
  - Reads the index file again after file is appended.
  - Does not allow for duplicate directories.
  - Checks that the directory actually exists.
  
- \-add filetype *extension*;*extension*;*...*
  - *extension* is in the format of .*extension*. For example, .exe.
  - Reads the filetypes file again after file is appended.
  - Does not allow for duplicate extensions.
  
 - \-list
   - Lists all the files that have been indexed.
  
 - \-index
   - Lists all the directories that are in the index file.
  
 - \-reload
   - Reads the index file and filetypes file.
   - Walks through all the directories and indexes files that have 
    an extension predefined by the user.
    
- \-quit
  - Quits the program.
    
# Files
- Files are stored in %APPDATA%/Finder as index.txt and filetypes.txt.
   - If files are edited manually, user needs to do -reload for the changes to take affect.
   
# Usage
- After files are indexed, user can search by typing in the name of the program.
- If that program is the only one with that name, the program is launched.
- Otherwise, a list of any programs containing the name is shown.
- If multiple programs are listed, the user can just press enter to select the top one.

- Once a program is selected, the window disappears. Pressing CTRL+ALT+F will bring the window back, ready to search again.
