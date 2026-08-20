---
description: Analysiere Änderungen, generiere ausführlichen Changelog und starte ESPHome Build für Büro
---

# Workflow: Release Build

Wenn dieser Workflow aufgerufen wird, führst du als KI-Assistent vollautomatisch die Vorbereitung und den Start des lokalen Firmware-Builds und Git-Releases durch.

## Ablauf

1. **Änderungen analysieren:**
   Führe per Terminal die Befehle `git log -n 3 --oneline` sowie `git status -s` und `git diff HEAD` aus. Analysiere präzise, welche Code-, Konfigurations- und Dokumentationsänderungen vorgenommen wurden.
2. **CHANGELOG.md analysieren (Ressourcenschonend):**
   Nutze das Terminal (`run_command`), um mit `head -n 50 CHANGELOG.md` ausschließlich die obersten 50 Zeilen der Changelog-Datei zu lesen (die neuesten Releases stehen oben). **Lies auf keinen Fall die komplette Datei ein**, um Tokens zu sparen. Verstehe anhand der letzten Einträge das genaue Format (`Keep a Changelog`).
3. **Dateien aktualisieren:**
   * **version.json**: Erstelle einen kurzen, prägnanten englischen Satz (ca. 5–12 Wörter), der die Hauptänderung zusammenfasst. Überschreibe damit den Wert des `"project_description"`-Feldes in `version.json`.
     **⚠️ KRITISCHE REGEL:** Erhöhe **NIEMALS** manuell den Wert für `"project_version"` in der Datei. Der Version-Bump geschieht **vollautomatisch** durch `python3 version_bump.py` vor dem Build bzw. durch den Git `pre-commit`-Hook. Ein manuelles Editieren führt zu einem fehlerhaften "Double-Bump".
   * **CHANGELOG.md**: Schreibe einen **ausführlichen, detaillierten** Changelog-Eintrag zu deinen analysierten Änderungen (unterteilt in `Added`, `Changed`, `Fixed`, `Removed`). Setze als Versionsnummer im Titel (z.B. `## [0.9.26] - YYYY-MM-DD`) die um 1 erhöhte Patch-Version aus `version.json` ein. Füge diesen Eintrag direkt unter dem Header oben in die `CHANGELOG.md` ein.
4. **Kompilierung & Git Push:**
   Generiere eine passende englische Conventional-Commit-Nachricht (z.B. `feat: ...`, `fix: ...` oder `chore: ...`).
   Führe den Versions-Bump aus, starte den Firmware-Build/Upload und pushe bei Erfolg automatisch ins Git-Repository.
   // turbo
   Führe den folgenden Befehl im Terminal aus (ersetze `DEINE_MESSAGE` durch deine generierte Commit-Nachricht):
   `rm -f .version_bump_lock && python3 version_bump.py && esphome run ventosync_nosensor.yaml --device 192.168.178.206 --no-logs && git add . && git commit -m "DEINE_MESSAGE" && git push`
   Setze `SafeToAutoRun` auf `true`.

Zeige dem User abschließend kurz die generierte `project_description`, den neuen `CHANGELOG.md`-Eintrag sowie die Commit-Nachricht an und melde, dass der Build inkl. anschließendem Push gestartet wurde.
