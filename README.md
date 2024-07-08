> Tessellation auf Terrain führt zu gelegentlichen Assertion-Fails im LandscapeRenderer im cooked Build und damit zu Fatal Errors im Shipment!

# Viking-Mage
Unreal-Repository für unser Projekt mit dem Arbeitstitel "Viking Mage".

## Umgang mit dem Repository
Um uns allen das Leben möglichst leicht zu machen und effizient arbeiten zu können, muss sich an gewissen Vorgaben zum Umgang mit dem Repository und dem Ablegen von Dateien gehalten werden. Damit wir alle einheitlich auf einem Stand sind, halten sich bitte alle an folgenden Style-Guide: https://github.com/Allar/ue4-style-guide#structure . 

### Ordnerstruktur
#### Beispiel:
<pre>
|-- Content
    |-- GenericShooter
        |-- Art
        |   |-- Industrial
        |   |   |-- Ambient
        |   |   |-- Machinery
        |   |   |-- Pipes
        |   |-- Nature
        |   |   |-- Ambient
        |   |   |-- Foliage
        |   |   |-- Rocks
        |   |   |-- Trees
        |   |-- Office
        |-- Characters
        |   |-- Bob
        |   |-- Common
        |   |   |-- Animations
        |   |   |-- Audio
        |   |-- Jack
        |   |-- Steve
        |   |-- Zoe
        |-- Core
        |   |-- Characters
        |   |-- Engine
        |   |-- GameModes
        |   |-- Interactables
        |   |-- Pickups
        |   |-- Weapons
        |-- Effects
        |   |-- Electrical
        |   |-- Fire
        |   |-- Weather
        |-- Maps
        |   |-- Campaign1
        |   |-- Campaign2
        |-- MaterialLibrary
        |   |-- Debug
        |   |-- Metal
        |   |-- Paint
        |   |-- Utility
        |   |-- Weathering
        |-- Placeables
        |   |-- Pickups
        |-- Weapons
            |-- Common
            |-- Pistols
            |   |-- DesertEagle
            |   |-- RocketPistol
            |-- Rifles
</pre>

#### Was bedeutet das für uns konkret? 
Zusätzlich zur oben dargestellten Struktur gibt es die `Sandbox`. Häufig könnte dafür auch der `Developer`-Ordner verwendet werden, allerdings liegt dieser nur lokal auf eurem Rechner und nicht im Repository vor! Möchtet ihr also ein Asset nutzen, dass **nicht** hochgeladen werden soll, nutzt bitte den `Developer`-Ordner. Alle Experimente, die **nicht im finalen Build** sein sollen und nicht abgenommen wurden, werden bitte unter `Content/Sandbox/NameDesEntwicklers` durchgeführt. </br>
Assets von Dritten können direkt in den `Content`-Ordner abgelegt werden. Eigene Assets werden nach obiger Struktur im Projekt-Ordner oder in der `Sandbox` abgelegt. Beachtet hier den Punkt "Commits/Pull Requests" </br>

#### Nutzung von Assets aus der Sandbox
Es ist erlaubt die Assets aus der Sandbox anderer Entwickler zu nutzen. Kontaktiert denjenigen davor aber! Assets in der Sandbox können sich ohne Rücksprache jederzeit ändern, was wiederrum zu Problemen in euren eigenen Szenen führen kann. </br>
Wollt ihr sicher sein, dass die Assets ohne Probeleme funktionieren, nutzt die offiziellen Assets aus dem Projektordner. </br>

### Commits/Pull Requests
Bevor etwas in den Main-Branch committed wird erstellt euch bitte einen eigenen Branch und erstellt einen Pull-Request, der überprüft werden kann. Dann kann euer Branch mit dem Main-Branch verbunden werden. (Mehr Infos dazu in der Gitkraken Doku). 

#### Ergänzungen/Veränderungen von Assets im Projektordner
Assets im Projektordner (also nicht in der Sandbox) werden **niemals** direkt auf dem Mainbranch geändert! Müssen Assets dort verändert oder bearbeitet werden, wird ein **neuer Branch** erstellt, der wie oben beschrieben mit einem `Pull request` commited wird. Hier kann dann zuerst die Funktionalität geprüft werden und wenn alles ohne Probleme funktioniert findet der `merge` mit dem `Main branch` statt. 

#### Probleme mit GitKraken bei Großen/Vielen Dateien?
GitKraken unterbricht den Upload ggf. weil der Token für Github abgelaufen ist. Alternativ kann z.B. GitHub Desktop genutzt werden. Dort sieht man auch einen Upload-Zustand. Ein Commit kann trotzdem über GitKraken vorbereitet und dann über GitHub Desktop gepushed werden. (https://desktop.github.com/) </br>

## Code-Richtlinien
Wesentliche Code-Richtlinien finden sich in dem oben verlinkten [StyleGuide](https://github.com/Allar/ue4-style-guide).
### Sprache
#### Dokumentation
Jegliche Dokumentation, d.h. in der Wiki, Tooltips in Parametern, Inline-Kommentare, ... erfolgen auf **Deutsch**.
#### Im Code
Die Sprache im Code selber wird auf **Englisch** gehalten. D.h. jegliche Bezeichner sind englisch. 

## Ergänzungen zur `Readme.md`
Möchtet ihr hier etwas ergänzen, haltet euch bitte an folgenden Markdown-Stylguide: https://www.markdownguide.org/basic-syntax/ . 
