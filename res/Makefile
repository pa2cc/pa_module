certs:
#	Generates the key
	openssl req -x509 -newkey rsa:2048 -nodes -keyout localhost.key -out localhost.crt -days 3650 -subj '/CN=localhost/O=PACC'
	certutil -d sql:$$HOME/.pki/nssdb -D -n PACC
	certutil -d sql:$$HOME/.pki/nssdb -A -t "C" -n PACC -i localhost.crt
