<?php
    // Connexion à la base de données
    $host = "localhost";
    $dbname = "parking_db";
    $username = "postgres";
    $password = "admin";
	$port = 5432;
	$dsn = "pgsql:host=$host;port=$port;dbname=$dbname";
    $options = [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION];

    try {
        $pdo = new PDO($dsn, $username, $password, $options);
    } catch (PDOException $e) {
        die("Erreur de connexion à la base de données : " . $e->getMessage());
    }
?>