<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="../styles.css">
    <title>Sum Result</title>
</head>
<body>
    <div class="container">
        <h1>Sum Result</h1>
        <?php
        if(isset($_POST['num1']) && isset($_POST['num2'])){
            $num1 = $_POST['num1'];
            $num2 = $_POST['num2'];
            $sum = $num1 + $num2;
            echo "<p>The sum of $num1 and $num2 is: $sum</p>";
        } else {
            echo "<p>Numbers were not provided for calculation.</p>";
        }
        ?>
        <a href="index.php">Go Back</a>
    </div>
</body>
</html>
