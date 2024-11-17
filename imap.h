#ifndef IMAP_H
#define IMAP_H

/// handles the whole process of downloading messages
/// - logging in to the server
/// - selecting mailbox
/// - fetching UIDs of messages
/// - fetching the header/content of each message
/// - logging out
void download_messages(void);

#endif // !IMAP_H
