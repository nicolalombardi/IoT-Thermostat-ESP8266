const char INDEX[] PROGMEM = R"=====(
    <!DOCTYPE html>
    <html>
    	<head>
    		<title>Insert credentials</title>
    		<style>
    			.button{
    				color: white;
    				background-color: #1266EE;
    				height: 50px;
    				width: 200px;
    				border: 0 none;
    				border-radius: 5px;
    				margin: 5px;
    			}
    			.title{

    			}
    			.container{
    				text-align: center;
    				font-family: "Trebuchet MS", Helvetica, sans-serif;
    			}
    			.input{
    				padding:5px;
    				border:2px solid #ccc;
    				border-radius: 5px;
    				margin:2px;
    			}

    		</style>
    	</head>

    	<body>
    		<div class="container">
    			<h1 class="title">Thermostat</h1>
    			<h3>Insert your WiFi credentials</h3>

    			<form action="setCredentials" method="POST">
    				SSID:<br>
    				<input type="text" class="input" name="ssid" maxlength="32" required><br>
    				Password:<br>
    				<input type="password" class="input" id="password" name="password" maxlength="64" required><br>
    				<input type="checkbox" onchange="document.getElementById('password').type = this.checked ? 'text' : 'password'"> Show password <br><br>
    				<input type="submit" class="button" value="SUBMIT">
    			</form>
    		</div>
    	</body>
    </html>
)=====";

const char CREDSET[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <title>Credentials set</title>
    <style>
        .container{
            text-align: center;
            font-family: "Trebuchet MS", Helvetica, sans-serif;
        }
    </style>
  </head>

  <body>
      <div class="container">
        <h1>Credentials set correctly</h1>
      </div>
  </body>
</html>
)=====";
