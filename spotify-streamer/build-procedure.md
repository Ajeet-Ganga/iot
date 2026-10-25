# Spotify Streamer — Step by Step Build Procedure

**Setup route:** Mac → Raspberry Pi OS Lite → SSH → USB audio → Raspotify. Instructions checked against the linked project documentation on 25 October 2026; this exact hardware assembly has not yet been physically tested.

## Before starting

- Confirm that the speaker has its own power/amplifier and a 3.5mm AUX input. A passive speaker needs an amplifier; a Bluetooth-only speaker needs a different audio setup.
- Have Spotify Premium, home 2.4GHz Wi-Fi, a Mac and a microSD reader available. A monitor, HDMI cable, keyboard and soldering iron are not needed for this build.

## 1. Check the parts and speaker

Use one Pi Zero 2 W, one microSD card, power supply, OTG adapter, USB sound card and AUX cable. Fit the case if you have it. The speaker must be powered and set to AUX/Line In. Start with its volume low.

The 32GB card is sufficient; a reliable 16GB card would also be enough for this OS Lite build. No music library needs to be stored locally.

## 2. Write Raspberry Pi OS Lite on your Mac

Install Raspberry Pi Imager from https://www.raspberrypi.com/software/ and connect the microSD reader to the Mac. Back up the card first.

Select **Raspberry Pi Zero 2 W** as the device and **Raspberry Pi OS Lite (64-bit)**, the current stable release offered for it. Select the microSD carefully; writing will erase the selected drive.

In Imager's OS customisation, set hostname `spotify-streamer`, choose your own username and password, configure your 2.4GHz Wi-Fi SSID/password and country India, and enable SSH. Keep credentials private; do not put them in this document. Use password authentication for this local setup or an SSH key if already familiar with it.

Write the image, allow verification to finish, then eject the card. Imager screen labels may vary by version.

## 3. Assemble with the power disconnected

Insert the flashed microSD into the Pi and fit the case.

- **Audio:** Pi socket labelled `USB` → Micro-USB OTG adapter → USB sound card → sound card headphone/output socket → AUX cable → speaker AUX input.
- **Power:** wall socket → official 5.1V / 2.5A supply → Pi socket labelled `PWR IN`.

The ₹83 OTG adapter is for USB audio data, not for the power input. The power supply's attached Micro-USB lead connects directly to `PWR IN`. Use the sound card's output, not its microphone socket.

## 4. Boot and connect from the Mac

Turn on the speaker, select AUX, then power the Pi. Allow several minutes for first boot. Put the Mac and phone on the same home network as the Pi.

In Mac Terminal, replace `YOUR_USERNAME` with the username selected in Imager:

```bash
ssh YOUR_USERNAME@spotify-streamer.local
```

On the first connection, verify this is your new Pi, accept its host key, and enter your password. Terminal does not display password characters.

If the hostname does not resolve, find the Pi's IP address in the router's connected-device list, then use `ssh YOUR_USERNAME@PI_IP_ADDRESS`. If the Pi never joins Wi-Fi, check that a 2.4GHz network and the correct country/password were written in Imager.

## 5. Update the Pi and install audio utilities

Run these commands on the Pi through SSH, not directly on the Mac:

```bash
sudo apt update
sudo apt full-upgrade -y
sudo apt install -y curl alsa-utils
sudo reboot
```

Reconnect by SSH after reboot.

## 6. Identify and test the USB audio output

List attached playback devices:

```bash
aplay -l
aplay -L
```

Identify the USB sound card. From a line such as `card 1: Device [...], device 0: ...`, its stable card ID is `Device`, not the number `1`. Card names and numbers can differ between dongles.

For that example, test at low speaker volume:

```bash
speaker-test -D 'plughw:CARD=Device,DEV=0' -c 2 -t sine
```

Replace `Device` and the device number with the values shown on your Pi. You should hear alternating left/right tones; stop with Ctrl+C. If the card is muted, run `alsamixer`, press F6, select the USB card, and unmute its playback control with M if necessary. Start at a moderate level.

Do not continue until this test produces sound. If no USB card appears, check the OTG adapter is in `USB`, not `PWR IN`, and reseat the dongle.

## 7. Install Raspotify

Raspotify provides a Spotify Connect receiver using librespot. It requires Spotify Premium. It is a community implementation rather than a Spotify-certified appliance; upstream changes can require software updates.

Use the maintainer's installer. Download it first so you can inspect it before running:

```bash
curl -fL https://dtcooper.github.io/raspotify/install.sh -o /tmp/raspotify-install.sh
less /tmp/raspotify-install.sh
```

Press `q` to leave the viewer, then run:

```bash
sudo sh /tmp/raspotify-install.sh
```

If the download or installer fails, stop and check the error against the official repository rather than using an unrelated installation script. Use https://github.com/dtcooper/raspotify as the maintained reference.

## 8. Configure the receiver and USB output

Open the configuration:

```bash
sudo nano /etc/raspotify/conf
```

Edit existing settings rather than leaving duplicate active entries. Remove the leading `#` from settings you enable. Use the USB device identifier tested in step 6:

```ini
LIBRESPOT_NAME="Spotify Streamer"
LIBRESPOT_BACKEND=alsa
LIBRESPOT_DEVICE="plughw:CARD=Device,DEV=0"
LIBRESPOT_BITRATE=320
LIBRESPOT_INITIAL_VOLUME=30
```

Keep the shipped audio-cache-disable setting enabled to avoid unnecessary storage writes. Leave discovery enabled; do not uncomment `LIBRESPOT_DISABLE_DISCOVERY`. Do not enter a Spotify password in the configuration—password authentication is no longer supported; the normal setup discovers the receiver through the Spotify app.

Save in nano with Ctrl+O, Enter, then Ctrl+X. Config options can change; consult the installed file's comments if they differ from this guide.

## 9. Start the service

Enable startup and restart the receiver:

```bash
sudo systemctl enable --now raspotify
sudo systemctl restart raspotify
systemctl status raspotify --no-pager
```

Expected result: `active (running)`. If it failed, read recent logs:

```bash
sudo journalctl -u raspotify -b -n 80 --no-pager
```

If the audio test says the device is busy after installation, stop Raspotify before retesting, then restart it:

```bash
sudo systemctl stop raspotify
```

## 10. Play Spotify and verify the result

Open Spotify on your phone using your Premium account. Keep it on the same local network for discovery, start a track, open the device picker / Connect to a device, and select **Spotify Streamer**.

Raise volume gradually. Try pause/resume, changing tracks, and adjusting volume from the phone. The Pi receives the stream; the phone controls it.

Reboot the Pi once and confirm it automatically returns to the device list and can play again. No monitor or keyboard should be needed for everyday use.

## 11. Troubleshoot common problems

- **Receiver missing:** confirm the Pi has Wi-Fi, Spotify Premium is active, and raspotify is running. Avoid guest Wi-Fi or router client isolation, which can block local discovery. Restart Spotify and the service.
- **Receiver visible but silent:** confirm AUX is selected, the cable is in the sound card's output socket, and `LIBRESPOT_DEVICE` matches the successful speaker-test device. Stop Raspotify before rerunning the tone test.
- **Distortion or hiss:** reduce the phone/receiver and speaker volumes, check cables and the power supply, then assess the inexpensive USB dongle. Upgrade audio hardware only if its noise is audible in normal use.
- **Random restarts or USB disconnects:** check the power connection and supply first. Never feed more than the specified 5V-class input through the Micro-USB power port.
- **Failed playback after a Spotify change:** check the Raspotify issue tracker and update available packages with `sudo apt update` followed by `sudo apt upgrade`. Follow the current maintainer guidance if a protocol change needs a newer release.

## 12. Shut down and record completion

Before removing power or the microSD, run:

```bash
sudo poweroff
```

Wait about 30 seconds before unplugging power. Do not pull the card while the Pi is running.

Record after testing: speaker model, final USB card ID, OS release, Raspotify version, test date, and whether reboot/startup playback passed. Keep a copy of the working configuration without credentials.

## Reference links

- Raspberry Pi Imager and OS downloads: https://www.raspberrypi.com/software/
- Raspberry Pi setup: https://www.raspberrypi.com/documentation/computers/getting-started.html
- SSH and remote access: https://www.raspberrypi.com/documentation/computers/remote-access.html
- Raspotify installation and support: https://github.com/dtcooper/raspotify
- Raspotify configuration reference: https://github.com/dtcooper/raspotify/blob/master/raspotify/etc/raspotify/conf
