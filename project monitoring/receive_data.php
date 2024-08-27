<?php
// Include database connection file
include 'database.php';

// Check if data is received
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Retrieve data from the POST request
    $voltage = $_POST['voltage'];
    $current = $_POST['current'];

    // Validate and sanitize input data
    $voltage = filter_var($voltage, FILTER_SANITIZE_NUMBER_FLOAT, FILTER_FLAG_ALLOW_FRACTION);
    $current = filter_var($current, FILTER_SANITIZE_NUMBER_FLOAT, FILTER_FLAG_ALLOW_FRACTION);

    // Prepare the SQL statement
    $stmt = $conn->prepare("INSERT INTO measurement (voltage, current) VALUES (?, ?)");
    $stmt->bind_param("dd", $voltage, $current);

    // Execute the statement and check for errors
    if ($stmt->execute()) {
        echo "Data inserted successfully";
    } else {
        echo "Error: " . $stmt->error;
    }

    // Close the statement and connection
    $stmt->close();
    $conn->close();
} else {
    echo "No data received";
}
?>
