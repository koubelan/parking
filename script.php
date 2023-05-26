<?php
// Connexion à la base de données
    include "db.php";

// Récupération des données envoyées par l'Arduino
$uid = $_POST['uid'];
$isVehicleDetectedSpot1 = $_POST['spot1'];
$isVehicleDetectedSpot2 = $_POST['spot2'];

// Recherche de l'employé dans la table "employees" en fonction de son UID
$query = "SELECT * FROM employees WHERE uid = :uid";
$stmt = $pdo->prepare($query);
$stmt->bindParam(':uid', $uid);
$stmt->execute();
$employee = $stmt->fetch(PDO::FETCH_ASSOC);

if ($employee) {
    // Mise à jour de la date de sortie de l'employé
    $query = "UPDATE employees SET date_out = NOW() WHERE uid = :uid";
    $stmt = $pdo->prepare($query);
    $stmt->bindParam(':uid', $uid);
    $stmt->execute();
} else {
    // Recherche d'un emplacement disponible dans la table "parking_spots"
    $query = "SELECT * FROM parking_spots WHERE is_available = true LIMIT 1";
    $stmt = $pdo->query($query);
    $spot = $stmt->fetch(PDO::FETCH_ASSOC);

    if ($spot) {
        // Insertion des données de l'employé dans la table "employees"
        $query = "INSERT INTO employees (uid, date_in, parking_spot) VALUES (:uid, NOW(), :parking_spot)";
        $stmt = $pdo->prepare($query);
        $stmt->bindParam(':uid', $uid);
        $stmt->bindParam(':parking_spot', $spot['spot_number']);
        $stmt->execute();

        // Mise à jour de la disponibilité de l'emplacement dans la table "parking_spots"
        $query = "UPDATE parking_spots SET is_available = false WHERE id = :spot_id";
        $stmt = $pdo->prepare($query);
        $stmt->bindParam(':spot_id', $spot['id']);
        $stmt->execute();
    }
}

// Fermeture de la connexion à la base de données
$pdo = null;
?>
