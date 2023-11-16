<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="styles.css">
    <title>Add Two Numbers</title>
</head>
<body>
    <div class="container">
        <h1>Add Two Numbers</h1>
        <form action="./calculator/cal.php" method="post">
            <label for="num1">Number 1:</label>
            <input type="number" id="num1" name="num1" required>
            <br>
            <label for="num2">Number 2:</label>
            <input type="number" id="num2" name="num2" required>
            <br>
            <button type="submit">Calculate Sum</button>
        </form>
    </div>
</body>
</html>
