#ifndef HTTP_CONN_H
#define HTTP_CONN_H
#include <sys/epoll.h>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"
#include <sys/uio.h>
void addfd(int epollfd, int fd, bool one_shot, bool listen = false);
void removefd(int epollfd, int fd);
void modfd(int epollfd, int fd, int ev);
class http_conn
{
public:
    static int m_epollfd;
    static int m_user_count;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 2048;
    static const int FILENAME_LEN = 200;
    // HTTP���󷽷�������ֻ֧��GET
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT
    };

    /*
        �����ͻ�������ʱ����״̬����״̬
        CHECK_STATE_REQUESTLINE:��ǰ���ڷ���������
        CHECK_STATE_HEADER:��ǰ���ڷ���ͷ���ֶ�
        CHECK_STATE_CONTENT:��ǰ���ڽ���������
    */
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };

    /*
        ����������HTTP����Ŀ��ܽ�������Ľ����Ľ��
        NO_REQUEST          :   ������������Ҫ������ȡ�ͻ�����
        GET_REQUEST         :   ��ʾ�����һ����ɵĿͻ�����
        BAD_REQUEST         :   ��ʾ�ͻ������﷨����
        NO_RESOURCE         :   ��ʾ������û����Դ
        FORBIDDEN_REQUEST   :   ��ʾ�ͻ�����Դû���㹻�ķ���Ȩ��
        FILE_REQUEST        :   �ļ�����,��ȡ�ļ��ɹ�
        INTERNAL_ERROR      :   ��ʾ�������ڲ�����
        CLOSED_CONNECTION   :   ��ʾ�ͻ����Ѿ��ر�������
    */
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    // ��״̬�������ֿ���״̬�����еĶ�ȡ״̬���ֱ��ʾ
    // 1.��ȡ��һ���������� 2.�г��� 3.���������Ҳ�����
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

    HTTP_CODE process_read();
    HTTP_CODE parse_request_line(char *);
    HTTP_CODE parse_headers(char *);
    HTTP_CODE parse_content(char *);
    HTTP_CODE do_request();
    LINE_STATUS parse_line();
    http_conn(){};
    ~http_conn(){};
    void process();
    void init(int sockfd, const sockaddr_in &addr);
    void close_conn();
    bool write();
    bool read();

private:
    int m_sockfd;
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_content_type();
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

    bool process_write(HTTP_CODE ret);
    int m_checked_index;
    int m_start_line;
    char *m_url;
    char *m_version;
    char *m_host;
    int m_content_length;
    bool m_linger;
    METHOD m_method;
    char m_real_file[FILENAME_LEN];
    char *m_file_address;
    struct stat m_file_stat;
    void init();
    CHECK_STATE m_check_state;
    sockaddr_in m_address;

    char m_read_buf[READ_BUFFER_SIZE];
    int m_read_idx; //��ʶ������������Ŀͻ��˵����һ���ֽڵ���һ���ֽ�
    char *get_line()
    {
        return (m_read_buf + m_start_line);
    }
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    struct iovec m_iv[2];
    int m_iv_count;

    int bytes_to_send;
    int bytes_have_send;
};

#endif