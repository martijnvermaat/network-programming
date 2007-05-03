<?php


// Configuration
include('site.php');


function page_header($title = '') {

    if ($title == '') {
        $title = 'Conference Website';
        $back_link = '';
    } else {
        $title = 'Conference Website - '.$title;
        $back_link = '<p><a href="'.$BASEPHP.'index.php">Back to the Conference Website homepage</a></p>';
    }

    $doc_type = '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"'
        .' "http://www.w3.org/TR/html4/strict.dtd">';
    $content = <<<HTML
<html lang="en">
<head>
    <title>$title</title>
</head>
<body>
    <h1>Conference Website</h1>
    $back_link
    <hr>
HTML;
    return $doc_type.$content;

}


function page_footer() {
    $date = date('r');
    return <<<HTML
<hr>
<address>Conference Website - $date</address>
</body>
</html>
HTML;
}


// Prepare an error page for given message string
function error_page($message) {

    return
        page_header('Error')
        .'<h2>An error occured</h2>'
        .'<pre>'.htmlentities($message).'</pre>'
        .'<p><a href="javascript:document.location.reload()">Try again</a></p>'
        .'<p><a href="javascript:history.go(-1)">Back to the previous page</a></p>'
        .page_footer();

}


function parse_response($response) {

    $lines = explode("\n", trim($response));

    $response = array('content' => array_slice($lines, 1));

    if (!preg_match('/^([0-2]) ([^\n]*)$/', $lines[0], $m)) {
        $response['status'] = 2;
        $response['message'] = 'Malformed response';
    } else {
        $response['status'] = intval($m[1]);
        $response['message'] = $m[2];
    }

    return $response;

}


function parse_list_response($response) {

    if ($response['status'] != 0) {
        return $response;
    }

    for ($i = 0; $i < count($response['content']); $i++) {
        if (!sscanf($response['content'][$i], "%s %f %i", $type, $price, $number)) {
            $response['status'] = 2;
            $response['message'] = 'Malformed response';
            break;
        }
        $response['content'][$i] = array('type'   => $type,
                                         'price'  => $price,
                                         'number' => $number);
    }

    return $response;

}


function connect_to_gateway($address, $port) {

    if (FALSE === ($socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP))) {
        die(error_page("Could not create socket:\n".socket_strerror(socket_last_error())));
    }

    if (FALSE === @socket_set_option($socket, SOL_SOCKET, SO_REUSEADDR, 1)) {
        die(error_page("Could not set socket option (SO_REUSEADDR):\n".socket_strerror(socket_last_error())));
    }

    if (FALSE === @socket_bind($socket, $_SERVER['SERVER_ADDR'])) {
        die(error_page("Could not bind socket:\n".socket_strerror(socket_last_error())));
    }

    if (FALSE === @socket_connect($socket, $address, $port)) {
        die(error_page('Could not connect to hotel service at '.$address.' (port '.$port."):\n".socket_strerror(socket_last_error())));
    }

    return $socket;

}


function send_request($socket, $request) {

    $r = $request['procedure']."\n";
    $r .= implode("\n", $request['parameters'])."\n";

    $total_sent = 0;

    while ($total_sent < strlen($r)) {
        if (FALSE === ($sent = @socket_write($socket, substr($r, $total_sent)))) {
            die(error_page("Could not send request to hotel service:\n".socket_strerror(socket_last_error())));
        }
        if ($sent == 0) {
            die(error_page('Could not send request to hotel service'));
        }
        $total_sent += $sent;
    }

    $response = '';

    do {
        if (FALSE === ($read = @socket_read($socket, 255))) {
            die(error_page("Could not read response from hotel service:\n".socket_strerror(socket_last_error())));
        }
        $response .= $read;
    } while ($read != '');

    return parse_response($response);

}

?>
