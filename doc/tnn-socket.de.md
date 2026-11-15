# TNT mit TNN über einen Unix-Socket verbinden

Diese Anleitung beschreibt die Verbindung von TNT zu einem TNN, das einen
Unix-Domain-Socket für sein Hostmode-Interface bereitstellt. TNT spricht dort
das TFKISS-Hostmode-Protokoll. Ein normaler TCP-Port oder ein Socket für
Anwendungsdienste ist kein Ersatz dafür.

## 1. TNN vorbereiten

Richte TNN so ein, dass es einen lokalen Hostmode-Socket bereitstellt, und
notiere dessen vollständigen Pfad. Der konkrete TNN-Konfigurationsschalter
dafür hängt von der verwendeten TNN-Version ab; beachte dazu die TNN-Doku.

Beispielpfad:

```text
/run/tnn-ham/tnn-socket
```

Der Benutzer, unter dem TNT läuft, muss auf den Socket zugreifen können.
Starte TNN vor TNT, damit der Socket beim Start von TNT bereits existiert.

## 2. TNT konfigurieren

Trage in der von TNT verwendeten `tnt.ini` den Soft-TNC-Modus und den
Socket-Pfad ein. Bei der Standardkonfiguration liegt die Datei unter
`~/.tnt/tnt.ini`; TNT kann alternativ mit `-i` eine andere Konfigurationsdatei
verwenden.

```ini
soft_tnc 1
device /run/tnn-ham/tnn-socket
tnc_channels 10
```

Ersetze den Beispielpfad durch den von TNN angelegten Socket und setze
`tnc_channels` passend zur Zahl der von TNN bereitgestellten Kanäle. Bei
`soft_tnc 1` erwartet TNT einen Unix-Domain-Hostmode-Socket.

TNT muss mit Soft-TNC-/Serial-Unterstützung gebaut sein. Wenn du eine
Minimalkonfiguration verwendest, darf `TNT_SERIAL` nicht deaktiviert sein.

## 3. TNT starten und verbinden

Starte zuerst TNN und danach TNT unter einem Benutzer, der den Socket
verwenden darf. Wähle in TNT einen freien Kanal und starte wie gewohnt einen
Connect. Die Connects und die Funk-/AX.25-Routen werden weiterhin von TNN
verwaltet; TNT stellt die Kanäle und die Terminaloberfläche bereit.

## 4. Fehler eingrenzen

- **TNT meldet, dass es den Soft-TNC-Socket nicht verbinden kann:** Prüfe,
  ob TNN läuft, der Pfad exakt stimmt und der TNT-Benutzer Zugriff hat.
- **Der Socket existiert, aber es kommt keine Hostmode-Kommunikation zustande:**
  Stelle sicher, dass es TNNs Hostmode-Schnittstelle mit dem erwarteten
  TFKISS-Protokoll ist, nicht ein anderer TNN-Dienstsocket.
- **TNT verbindet, aber Kanäle oder Connects funktionieren nicht:** Vergleiche
  `tnc_channels` mit der TNN-Kanal-Konfiguration und prüfe Rufzeichen,
  Funkgerät und Routing in TNN.

`SOCKCon <Adresse>` ist nicht für diese Verbindung gedacht: Dieser TNT-Befehl
verbindet einen TNT-Kanal mit einem beliebigen Anwendungs-Socket. Ebenso
startet `SOCKET AXSERV ...` einen Server in TNT, anstatt TNT mit TNNs
Hostmode-Socket zu verbinden.
