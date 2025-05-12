<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);

header('Content-Type: application/json');

// DB info
$host = "212.237.182.132";
$db = "caravan_datalogger";
$user = "caravan";
$pass = "CaravansAreAwesome"; 

$conn = new mysqli($host, $user, $pass, $db);
if ($conn->connect_error) {
    http_response_code(500);
    echo json_encode(["error" => "DB connection failed"]);
    exit;
}

// Hent og konverter POST data
$timestamp = $_POST['timestamp'] ?? '';  // Skal være en datetime streng (f.eks. 2025-05-12 06:21:18)
$lat       = floatval($_POST['lat'] ?? 0);
$lon       = floatval($_POST['lon'] ?? 0);
$speed     = isset($_POST['speed']) ? floatval($_POST['speed']) : NULL;
$alt       = isset($_POST['alt']) ? floatval($_POST['alt']) : NULL;
$temp_in   = isset($_POST['temp_in']) ? floatval($_POST['temp_in']) : NULL;
$hum_in    = isset($_POST['hum_in']) ? floatval($_POST['hum_in']) : NULL;
$temp_out  = isset($_POST['temp_out']) ? floatval($_POST['temp_out']) : NULL;
$hum_out   = isset($_POST['hum_out']) ? floatval($_POST['hum_out']) : NULL;

// Valider påkrævede data
if (!$timestamp || !$lat || !$lon) {
    http_response_code(400);
    echo json_encode(["error" => "Missing required fields"]);
    exit;
}

// Forbered og udfør indsættelse
$stmt = $conn->prepare("INSERT INTO caravan_data (timestamp, lat, lon, speed, alt, caravan_temp_in, caravan_hum_in, caravan_temp_out, caravan_hum_out)
                        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

if (!$stmt) {
    http_response_code(500);
    echo json_encode(["error" => "Prepare failed: " . $conn->error]);
    exit;
}

$stmt->bind_param("sdddddddd", $timestamp, $lat, $lon, $speed, $alt, $temp_in, $hum_in, $temp_out, $hum_out);

if ($stmt->execute()) {
    echo json_encode(["status" => "ok"]);
} else {
    http_response_code(500);
    echo json_encode(["error" => "DB insert failed: " . $stmt->error]);
}

$stmt->close();
$conn->close();

?>
