To run the program you can use the command make all.

USAGE: ./proc <dial_id>

make run: executes the program as ./proc 2 (it creates/enters the conversation with id = 2)

To create another conversation, you simply run ./proc and provide a dial_id that is not currently active.

To join an existing conversation, you provide as dial_id the number of the conversation you want to enter, which must already have been created by another process.

For example, if we run ./proc 5 from 3 terminals:

The first time, a new conversation with dial_id = 5 will be created.
The second and third time, the processes will simply join the existing conversation and be able to exchange messages.

After joining, a participant can immediately type a message in stdin, and all processes participating in the conversation will print it to their stdout in the format:

Message received: <msg>

To terminate the conversation, type TERMINATE.

The design principles and more information are available in **report.pdf**.
