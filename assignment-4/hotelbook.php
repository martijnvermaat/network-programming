<?php


// Include some shared functionality
include('conference-shared.php');


$show_form = true;
$error = '';
$field_name = '';
$field_type = '';


if ($_SERVER['REQUEST_METHOD'] == 'POST') {

    // Handle booking

    $show_form = false;

    $field_type = $_POST['type'];
    $field_name = $_POST['name'];

    $request = "book\n";

    if (isset($_POST['type']) && preg_match('/^[a-z0-9_-]+$/i', $_POST['type'])) {
        $request .= $_POST['type']."\n";
    }

    if (!isset($_POST['name']) || strlen($_POST['name']) < 1) {

        $show_form = true;
        $error = 'You must provide a name in order to book a room';

    } else if (strpos($_POST['name'], "\n")) {

        $show_form = true;
        $error = 'Your name may not contain a newline character';

    } else {

        $request .= $_POST['name']."\n\n";

        // Send request

        $socket = connect_to_gateway($HOTEL_GATEWAY_ADDR, $HOTEL_GATEWAY_PORT);

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

        $booking = parse_response($response);

        if ($booking['status'] == 0) {
            echo page_header("Room Booked");
            echo '<h2>Room Booked Successfully</h2>';
            echo '<p>Thank you for your reservation.</p>';
            echo page_footer();
        } else if ($booking['status'] == 1) {
            $show_form = true;
            $error = $booking['message'];
        } else {
            die(error_page("Could not read response from hotel service:\n".$booking['message']));
        }

    }

}


if ($show_form) {

    // Show booking form

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

    echo page_header('Book a Room');

    if ($error != '') {
        echo '<h2>Booking Not Successful</h2>';
        echo '<p>'.htmlentities($error).'</p>';
    }

    echo '<h2>Book a Room</h2>';

    if (count($availabilities) < 1) {

        echo '<p>Unfortunately, there are no rooms available at this time.</p>';

    } else {

        if (isset($_GET['name'])) {
            $field_name = $_GET['name'];
        }
        if (isset($_GET['type'])) {
            $field_type = $_GET['type'];
        }

?>

<form method="post" action="<?= $BASEPHP?>hotelbook.php">
<dl>
<dt><label for="field-name">Your name</label></dt>
<dd><input type="text" name="name" id="field-name" value="<?= htmlentities($field_name) ?>"></dd>
<dt><label for="field-type">Room type</label></dt>
<dd>
    <select name="type" id="field-type">
    <?php
    foreach ($availabilities as $availability) {
        echo '<option value="'.htmlentities($availability['type']).'"';
        if ($availability['type'] == $field_type) {
            echo ' selected="selected"';
        }
        echo '>Room type '.htmlentities($availability['type']).' at &euro; '.sprintf('%.2f', $availability['price']).' per room';
    }
    ?>
     </select>
</dd>
</dl>
<p><input type="submit" value="Book room"></p>
</form>

<?php

    }

    echo page_footer();

}


?>
