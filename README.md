# 📸 Intervallomètre WiFi pour Sony RX100

### 📝 Description du projet
Ce projet est une solution matérielle autonome pour contrôler les caméras Sony (testé sur RX100M2 uniquement) via WiFi, contournant les limitations de l'application mobile propriétaire. Il a été réalisé grâce au reverse engineering de l'API Sony.

### ⚙️ Implémentation Technique
* **Hardware :** Arduino Nano ESP32 (ESP32-S3) + Écran LCD 16x2.
* **Connectivité :** L'ESP32 crée un **Portail Captif** (Mode AP). L'utilisateur configure le timelapse via une page web stockée dans la mémoire du microcontrôleur.
* **Communication :** Envoi de commandes JSON (`setShootMode`, `actTakePicture`) directement à l'API de la caméra.
* **Fonctionnalité :** Calcul automatique de la durée totale et du nombre de photos restantes.

### 📺 Démo Vidéo
[▶️ Voir la démonstration vidéo sur YouTube](#)

### 📄 Licence
Ce projet est sous licence **GNU GPLv3**. Vous êtes libres d'utiliser, modifier et redistribuer ce logiciel, à condition de créditer l'auteur et de conserver la même licence libre pour les travaux dérivés.
