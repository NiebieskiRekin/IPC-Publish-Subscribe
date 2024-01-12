#include "projekt.h"

int main(int argc, char* argv[])
{
    int logged = 0;
    int clients_mid[16];
    Client Logged_in[16];
    int server_mid = msgget(0x100, 0600 | IPC_CREAT);
    while (1)
    {
        if (msgrcv(server_mid, &m_login_user, 128, Login, IPC_NOWAIT)!=-1)
        {
            printf("%s", "ASDGSHD\n");
            strcpy(Logged_in[logged].name, m_login_user.name);
            Logged_in[logged].msgid = m_login_user.msgid;
            LoginStatus m_login_status = {.type = Login, .status = 1};
            strcpy(Logged_in[logged].name,m_login_status.name);
            clients_mid[logged] = msgget(Logged_in[logged].msgid, 0600 | IPC_CREAT);
            msgsnd(clients_mid[logged], &m_login_status, strlen(m_login_status.name)+1, IPC_NOWAIT);
            logged++;
        }
    }
}
