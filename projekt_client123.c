#include "projekt123.h"

int main(int argc, char* argv[])
{
    LoginStatus m_login_status = {Login, "", 0};
    int server_mid = msgget(0x100, 0600 | IPC_CREAT);
    key_t client_key = 0x123;
    LoginMessage m_login_user = {Login, "muj_nick", client_key};
    msgsnd(server_mid, &m_login_user, sizeof(LoginMessage), 0);
    printf("%s", "SEND\n");
    int cli_mid = msgget(client_key, 0600 | IPC_CREAT);
    msgrcv(cli_mid, &m_login_status, sizeof(LoginStatus), Login, 0);
    printf("%s %d\n", m_login_status.name, m_login_status.status);
}
