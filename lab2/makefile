all: conc_tcp_srv_echo conc_tcp_cli_echo tar

conc_tcp_srv_echo: conc_tcp_srv_echo.c
	gcc conc_tcp_srv_echo.c -o conc_tcp_srv_echo

conc_tcp_cli_echo: conc_tcp_cli_echo.c
	gcc conc_tcp_cli_echo.c -o conc_tcp_cli_echo

clean:
	rm -f conc_tcp_srv_echo.o conc_tcp_cli_echo.o conc_tcp_srv_echo conc_tcp_cli_echo conc_tcp_cs_echo.tar

tar:
	tar -cvf conc_tcp_cs_echo.tar conc_tcp_srv_echo.c conc_tcp_cli_echo.c