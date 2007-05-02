<?php


// Include some shared functionality
include('conference-shared.php');


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


$socket = connect_to_gateway($HOTEL_GATEWAY_ADDR, $HOTEL_GATEWAY_PORT);

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
        die(error_page("Could not read response from hotel service:\n".socket_strerror(socket_last_error())));
    }
    $response .= $read;
} while ($read != '');

$list = parse_list_response($response);

if ($list['status'] != 0) {
    die(error_page("Could not read response from hotel service:\n".$list['message']));
}

$availabilities = $list['content'];


echo page_header('Available Rooms');

echo '<h2>Available Rooms</h2>';

if (count($availabilities) < 1) {
    echo '<p>Unfortunately, there are no rooms available at this time.</p>';
}

echo '<dl>';
foreach ($availabilities as $availability) {
    echo '<dt>Rooms of type '.htmlentities($availability['type']).'</dt>';
    echo '<dd>'.htmlentities($availability['number']).' rooms available at &euro; '.sprintf('%.2f', $availability['price']).' per room';
    echo ' (<a href="'.$BASEPHP.'hotelbook.php?type='.htmlentities(urlencode($availability['type'])).'">book now!</a>)';
    echo '</dd>';
}
echo '</dl>';

echo page_footer();


?>
