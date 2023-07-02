all: iter_tcp_srv_echo iter_tcp_cli_echo tar

iter_tcp_srv_echo: iter_tcp_srv_echo.c
	gcc iter_tcp_srv_echo.c -o iter_tcp_srv_echo

iter_tcp_cli_echo: iter_tcp_cli_echo.c
	gcc iter_tcp_cli_echo.c -o iter_tcp_cli_echo

clean:
	rm -f iter_tcp_srv_echo.o iter_tcp_cli_echo.o iter_tcp_srv_echo iter_tcp_cli_echo iter_tcp_cs_echo.tar

tar:
	tar -cvf iter_tcp_cs_echo.tar iter_tcp_srv_echo.c iter_tcp_cli_echo.c