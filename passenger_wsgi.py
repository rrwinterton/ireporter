def application(environ, start_response):
    status = '200 OK'
    output = b'If you see this, Python is working perfectly!'
    response_headers = [
        ('Content-type', 'text/plain; charset=utf-8'),
        ('Content-Length', str(len(output)))
    ]
    start_response(status, response_headers)
    return [output]
