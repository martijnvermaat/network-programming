<?php


// Configuration
include('site.php');


// Prepare an error page for given message string
function error_page($message) {

    $title = 'Conference Website - Error';
    $message = htmlentities($message);
    $doc_type = '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"'
        .' "http://www.w3.org/TR/html4/strict.dtd">';
    $content = <<<HTML
<html lang="en">
<head>
    <title>$title</title>
</head>
<body>
    <h1>$title</h1>
    <p><a href="{$BASEPHP}index.php">Back to Conference Website</a></p>
    <hr>
    <h2>An error occured</h2>
    <pre>$message</pre>
</body>
</html>
HTML;
    return $doc_type.$content;

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

    $response = parse_response($response);

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


if (FALSE === ($socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP))) {
    die(error_page("Could not create socket:\n".socket_strerror(socket_last_error())));
}

if (FALSE === @socket_set_option($socket, SOL_SOCKET, SO_REUSEADDR, 1)) {
    die(error_page("Could not set socket option (SO_REUSEADDR):\n".socket_strerror(socket_last_error())));
}

if (FALSE === @socket_bind($socket, $_SERVER['SERVER_ADDR'])) {
    die(error_page("Could not bind socket:\n".socket_strerror(socket_last_error())));
}

if (FALSE === @socket_connect($socket, $HOTEL_GATEWAY_ADDR, $HOTEL_GATEWAY_PORT)) {
    die(error_page("Could not connect to hotel service:\n".socket_strerror(socket_last_error())));
}

$request = "list\n\n";
$total_sent = 0;

while ($total_sent < strlen($request)) {
    if (FALSE === ($sent = @socket_write($socket, substr($request, $total_sent)))) {
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
        die(error_page("Could not read request from hotel service:\n".socket_strerror(socket_last_error())));
    }
    $response .= $read;
} while ($read != '');

$list = parse_list_response($response);

if ($list['status'] != 0) {
    die(error_page("Could not read request from hotel service:\n".$list['message']));
}

$availabilities = $list['content'];


// Make sure this is the first thing we send
echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"';
echo ' "http://www.w3.org/TR/html4/strict.dtd">';

?>

<html lang="en">

<head>
    <title>Conference Website - Available Rooms</title>
</head>

<body>

<h1>Conference Website - Available Rooms</h1>

<p><a href="<?= $BASEPHP ?>index.php">Back to Conference Website</a></p>

<hr>

<h2>Available Rooms</h2>

<?php

if (count($availabilities) < 1) {
    echo '<p>Unfortunately, there are no rooms available at this time.</p>';
}

echo '<dl>';
foreach ($availabilities as $availability) {
    echo '<dt>Rooms of type '.htmlentities($availability['type']).'</dt>';
    echo '<dd>'.htmlentities($availability['number']).' rooms available at &euro; '.sprintf('%.2f', $availability['price']).' per room</dd>';
}
echo '</dl>';

?>

</body>

</html>
