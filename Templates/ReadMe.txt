The Templates directory was created for developers who either do not plan to use Visual Studio 6.0.  This directory provides "boilerplate" code for audio plug-ins and DXi synths, in MFC and non-MFC flavors.

To create you own plug-in from one of these templates, please do the following:
1. Copy one of the four subfolders to a new folder.
2. In this new folder, open the file named PlugInGUIDs.h into a text editor. 
3. Run GUIDGEN (provided with the Windows Platform SDK).
4. Paste 2 new GUIDs into the header file as indicated.
