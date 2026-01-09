# 📸 Intervallomètre WiFi pour Sony RX100

### 📝 Description du projet
Ce projet est une solution matérielle autonome pour contrôler les caméras Sony (testé sur RX100M2) via WiFi, contournant les limitations de l'application mobile propriétaire.

Le développement s'est appuyé sur l'analyse de l'API officielle Sony ainsi que sur du reverse engineering.

### 📚 Documentation Technique
Les documents officiels de l'API Sony utilisés pour ce projet sont inclus dans ce dépôt pour référence :
* **[Sony Camera Remote API - API Reference v2.40](docs/Sony%20Camera%20Remote%20API%20-%20API%20Reference%20v2.40.pdf)** : Détail des commandes JSON et des endpoints.
* **[Sony Camera Remote API Development Guide](docs/Sony%20Camera%20Remote%20API%20Development%20Guide.pdf)** : Guide de mise en œuvre du protocole.

### ⚙️ Implémentation Technique
* **Hardware :** Arduino Nano ESP32 (ESP32-S3) + Écran LCD 16x2.
* **Connectivité :** L'ESP32 crée un **Portail Captif** (Mode AP). L'utilisateur configure le timelapse via une page web stockée dans la mémoire du microcontrôleur (SPIFFS/LittleFS).
* **Protocole :** Envoi de requêtes HTTP POST avec des payloads JSON (ex: `setShootMode`, `actTakePicture`) directement vers l'API de la caméra.
* **Fonctionnalité :** Calcul automatique de la durée totale et feedback en temps réel sur l'écran LCD.

### 📺 Démo Vidéo
[▶️ Voir la démonstration vidéo sur YouTube](COLLE_TON_LIEN_YOUTUBE_ICI)

### 📄 Licence
Ce projet est sous licence **GNU GPLv3**. Vous êtes libres d'utiliser, modifier et redistribuer ce logiciel, à condition de me créditer et de conserver la même licence libre pour les travaux dérivés.
