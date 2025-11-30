Jason Li
14548692

# Question 1: TCP vs UDP - Why Stateful Communication Matters
Opening Statement:
TCP is necessary for this encrypted communication application because it is required to provide a reliable,
stateful connection that keeps the encryption process stable and consistent between client and server.

Key Points with Explanations:
1. When the client and server perform a key exchange using TCP, that key is there/active for the entire session.
Since TCP is connection orientated, everything that is sent after that key exchange happens through the same 
connection that was previously established. If we used UDP, every packet would act independently which we don't want 
because we would have no way of knowing if the messages came from the same client or session. We could track sessions, 
however that would be very inefficient than to just use TCP. For example in the assignment, we were allowed to use the same server 
with multiple clients and each client has it's own encrypted key, using TCP would just make it a lot more complicated. 
2. TCP also essentially guarantees that data arrives in order and without "loss". This is crucial for encryption because if a key exchange or encrypted message were lost or came out of order, the client and server would lose synchronization. With TCP, we can rely on the transport layer to handle all of that automatically, so our focus stays on the encryption logic itself.

Conclusion/Summary:
TCP keeps both sides in sync throughout the entire connection. Without it we would have to handle key management, 
message reliability, and ordering packets ourselves, which would not only make the system more complicated but inefficient as well.

# Question 2: Protocol Data Unit (PDU) Structure Design
Opening Statement:
We designed the protocol with PDU because it provides a consistent and reliable way to handle different types of data. 
It handles plaintext, encrypted messages, control commands, etc perfectly.

Key Points with Explanation:
1. If we just sent raw text like "ENCRYPT: THIS IS JASON LI," the reciever would have to parse strings, look for delimiters,
and look for the type of data that is being sent. That approach will work for simple text, but it breaks when dealing with encrypted btyes or binary data for example. The PDU structure solves this by defining the message type, direction, and payload length before the actual data is read. This way the reciever(server) always knows what to expect and how to interpret them. 
2. Using PDU structure also makes our protocol more efficient and easier to extend. For example, adding new message types such as digital signatures (the extra credit) only requires assigning a new "msg_type" value instead of changing the entire message format. It also avoids the common problems with special characters or null bytes that could appear in encrypted data.

Conclusion/Summary:
The fixed PDU structure makes our protocol predicable and easier to maintain and extend. It ensures every message can be 
read correctly even when sending complex data types. It also allows you to build upon and add new features without 
breaking compatibility. 

# Question 3: The Payload Length Field
Opening Statement:
Even though TCP guarantees reliable delivery, the payload_len field helps the reciever know exactly how much data belongs to one message. TCP sends data as a continous stream of bytes, not as seperate messages. 

Key Points with Explanation:
1. TCP doesn;t mark where one message ends and another begins. If we send multiple PDUs quickly, they can all arrive together
or get split into parts into mutliple recv() calls. The payload_len field tells the receiver(server) how many bytes to read for each message, so it will know when a full PDU has been received. Without it, the receiver would have to guess message sizes or look for special characters, which doesn't work for encrypted data. 

Conclusion/Summary:
Payload_len gives structure to the TCP stream. It lets us seperate and process messages correctly, now matter how TCP delivers the bytes.

# Question 4: Key Exchange Protocol and Session State
Opening Statement:
Generating new keys for each session is cruical because it keeps every client connection seperable/independent and secure. 
It also prevents old keys from being reused or exposed if one session is compromised. 

Key Points with Explanation:
1. If we used a shared key, every client and server would share the same encryption key. This means that if someone were to discover 
this key, they could decrypt every message. Creating new keys for each session ensures that even if one session is compromised, the other sessions aren't. 
2. When a TCP connection closes, the session and it's respective keys also ends. A new connection starts with a new key exchange.
This works with TCP's design as the connection itself defines the lifetime of the key. UDP doesn't have a persistent session which means it would lose the relationship between it's connection and key making security more difficult. 

Conclusion/Summary:
Generating new keys for each session improves security, prevents data leaks across connections, and ties encryption directly to the
TCP's session's lifecycle. This keeps communication secure. 

# Question 5: The Direction Field in the PDU Header
Opening Statement:
Technically we don't need it, however including the direction field adds clarity, safety, and flexibility to the protocol. 

Key Points with Explanation:
1. Including a direction field makes debugging and protocol validation easier. You can specifically check which messages are responses and which are requests. If the client or server sends the wrong message you can also detect and fix that error quickly. 
2. It also makes the protocol self documenting and easier to extend/build upon in the future. For example, hypothetically if we wanted to make this system into one where both the client and server can send requests (peer to peer system), the direction field already supports that feature. Without it, distinguishing message intent would be confusing and easy to make mistakes.

Conclusion/Summary:
The direction field improves readability, error detection, and future flexibility. It ensures that every message clearly defines its role in communication, making the protocol more robust and easier to maintain.