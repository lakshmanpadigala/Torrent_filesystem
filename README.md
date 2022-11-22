# AOS_ASSIGN_3

A group based file sharing system where users can share, download files from the group they belong to. Download can be parallel with multiple pieces from multiple peers.

Which can perform following operations.
\
1.Maintain information of clients with their files(shared by client) to assist the clients for the communication between peers
\
2. User can create an account and register with tracker
\
3. Login using the user credentials
\
4. Create Group and hence will become owner of that group
\
5. Fetch list of all Groups in server
\
6. Request to Join Group
\
7. Leave Group
\
8. Accept Group join requests (if owner)
\
9. Share file across group: Share the filename and SHA1 hash of the complete file as well as piecewise SHA1 with the tracker
\
10. Fetch list of all sharable files in a Group
\
11. Download file
  * a. Retrieve peer information from tracker for the file
  * b. Core Part: Download file from multiple peers (different pieces of file from different peers - piece selection algorithm) simultaneously and all the files which client downloads will be shareable to other users in the same group. Ensure file integrity from SHA1 comparison
12. Show downloads
13. Stop sharing file
14. Stop sharing all files(Logout)
15. Whenever client logins, all previously shared files before logout should automatically be on sharing mode

Can use following commands to perform the operations.\
a. Run Client: ```./client <IP>:<PORT> tracker_info.txt``` tracker_info.txt - Contains ip, port details of all the trackers\
b. Create User Account: ```create_user <user_id> <passwd>```\
c. Login: ```login <user_id> <passwd>```\
d. Create Group: ```create_group <group_id>```\
e. Join Group: ```join_group <group_id>```\
f. Leave Group: ```leave_group <group_id>```\
g. List pending join: ```requests list_requests <group_id>```\
h. Accept Group Joining Request: ```accept_request <group_id> <user_id>```\
i. List All Group In Network: ```list_groups```\
j. List All sharable Files In Group: ```list_files <group_id>```\
k. Upload File: ```upload_file <file_path> <group_id>```\
l. Download File: ```download_file <group_id> <file_name> <destination_path>```\
m. Logout: ```logout```\
n. Show_downloads: ```show_downloads```\
o. Stop sharing: ```stop_share <group_id> <file_name>```\
