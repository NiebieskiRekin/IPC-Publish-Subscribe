#include "projekt.h"

int main(int argc, char* argv[])
{

    int server_mid = msgget(0x100, 0600 | IPC_CREAT);

    LoginMessage m_login_user = {Login, "muj_nick", 0x123};
    msgsnd(server_mid, &m_login_user, strlen(m_login_user.name)+1, 0);
    printf("%s", "SEND\n");
    int mid = msgget(0x123, 0600 | IPC_CREAT);
    msgrcv(mid, &m_login_status, 128, Login, 0);
    printf("%s %d\n", m_login_status.name, m_login_status.status);
}
