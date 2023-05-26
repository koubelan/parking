<!DOCTYPE html>
<html>
<head>
    <title>Statut du parking</title>
    <link rel="stylesheet" type="text/css" href="styles.css">
</head>
<body>
    <h1>Statut du parking</h1>

    <h2>Emplacement disponible</h2>
    <?php
    // Connexion à la base de données
    include "db.php";

    // Recherche d'un emplacement disponible dans la table "parking_spots"
    $query = "SELECT * FROM parking_spots WHERE is_available = true";
    $stmt = $pdo->query($query);
    $spots = $stmt->fetchAll(PDO::FETCH_ASSOC);

    if (count($spots) > 0) {
        echo "<ul>";
        foreach ($spots as $spot) {
            echo "<li>Emplacement n°" . $spot['spot_number'] . "</li>";
        }
        echo "</ul>";
    } else {
        echo "<p>Tous les emplacements sont occupés.</p>";
    }

    // Fermeture de la connexion à la base de données
    $pdo = null;
    ?>

    <h2>Liste des employés ayant stationné leur voiture</h2>
    <table>
        <tr>
            <th>UID</th>
            <th>Date d'entrée</th>
            <th>Date de sortie</th>
            <th>Emplacement</th>
        </tr>
        <?php
        // Connexion à la base de données (à nouveau pour la deuxième requête)
        $pdo = new PDO($dsn, $username, $password, $options);

        // Récupération des données des employés depuis la table "employees"
        $query = "SELECT * FROM employees";
        $stmt = $pdo->query($query);
        $employees = $stmt->fetchAll(PDO::FETCH_ASSOC);

        foreach ($employees as $employee) {
            echo "<tr>";
            echo "<td>" . $employee['uid'] . "</td>";
            echo "<td>" . $employee['date_in'] . "</td>";
            echo "<td>" . $employee['date_out'] . "</td>";
            echo "<td>" . $employee['parking_spot'] . "</td>";
            echo "</tr>";
        }

        // Fermeture de la connexion à la base de données
        $pdo = null;
        ?>
    </table>
</body>
</html>
