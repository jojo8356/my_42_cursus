# Born2beRoot

*This project has been created as part of the 42 curriculum by jojo8356.*

---

## Description

Born2beRoot is the first system-administration project of the 42 common core. The
goal is not to write a program but to **build a machine**: a virtual server with
an encrypted disk, a strict password policy, a hardened SSH daemon, a firewall,
a monitoring script that talks to every logged-in user every ten minutes — and,
for the bonus part, a working WordPress site plus one useful extra service.

This repository contains the **complete, reproducible implementation** of the
subject (version 5.2):

| Path | What it is |
| --- | --- |
| `docs/en.subject.pdf` | The official subject, as published by 42 (18 pages, v5.2) |
| `docs/en.subject.md`, `docs/fr.subject.md` | Full transcriptions of the subject, English and French |
| `config/vm.conf` | **Every** project variable in one place: login, hostname, ports, LVM names, password rules, sudo rules, bonus settings |
| `config/partitions.layout` | The authoritative partition table (1 `/boot` + 7 LVM logical volumes inside a LUKS2 container) |
| `config/` | Ready-to-install configuration files (`sshd_config`, sudoers drop-in, `pwquality.conf`, UFW rules, MariaDB, lighttpd, vsftpd, fail2ban) |
| `scripts/setup.sh` | The orchestrator: runs the ten setup steps in order, with `--bonus`, `--dry-run`, `--quiet` and `--list` |
| `scripts/setup/*.sh` | One step per subject requirement (`00-system` … `90-bonus-service`) |
| `scripts/monitoring.sh` | The monitoring script, with the twelve mandatory fields |
| `scripts/verify.sh` | Self-check: 64 assertions against the live machine, exits non-zero on any failure |
| `scripts/launch.sh` | Creates and boots the virtual machine on a real host (VirtualBox or UTM) |
| `scripts/signature.sh` | Produces `signature.txt` — the SHA-256 of the virtual disk, as the subject requires |
| `tests/` | The full test suite: a VM emulator, per-requirement cases, and five community testers |

The two `README` languages (`README.md`, `README.fr.md`) are kept in sync; the
English one is authoritative.

---

## Instructions

### On a real machine (the way the subject is meant to be done)

The subject asks for **one** virtual machine, created by hand in VirtualBox
(Intel/AMD) or UTM (Apple Silicon), running the latest stable Debian without a
graphical server, with an encrypted LVM layout and a `signature.txt` file that
contains the SHA-256 of the disk.

```bash
# 1. Create the machine (VirtualBox example, 20 GiB disk, 2 GiB RAM)
bash scripts/launch.sh --provider virtualbox --iso ~/Downloads/debian-12.7.0-amd64-netinst.iso

# 2. Inside the fresh Debian install, copy this repository and run everything
sudo bash scripts/setup.sh --bonus

# 3. Check the result
sudo bash scripts/verify.sh

# 4. On the host, produce the signature the evaluator will diff
bash scripts/signature.sh ~/VirtualBox\ VMs/jojo8356/jojo8356.vdi
```

`scripts/setup.sh` is idempotent: it may be re-run at any time, and each step
prints `[ OK ]`, `[SKIP]` or `[FAIL]` for what it did.

### Without a machine (what this sandbox does)

Creating a real VM is impossible here (no `/dev/kvm`, no loop devices, no
`apt` network). The project therefore ships **a deterministic emulator** that
runs the real scripts against a real filesystem tree inside user namespaces:

```bash
cd born2beroot

# Boot a fresh machine, run the full setup, then the self-check
bash tests/vm.sh --snapshot /tmp/vm -- 'bash /opt/born2beroot/scripts/setup.sh --bonus'
bash tests/vm.sh --run      /tmp/vm -- 'bash /opt/born2beroot/scripts/verify.sh'

# The whole test suite: static checks, per-requirement cases, community testers
bash tests/run_tests.sh
```

The emulator gives the scripts a genuine environment — a real `/etc`, a real
dpkg database, a real LVM layout derived from `config/partitions.layout` — and
simulates only the parts the kernel would provide on a real machine
(`systemctl`, `ufw`, `crontab`, `mariadb`, `lighttpd`, `ss`). Every simulated
command keeps its real interface and its real output format, so the same
scripts, the same `verify.sh` and the same third-party testers run unmodified.

### The defence

The evaluator will ask for four things beyond the checks:

1. **Show `signature.txt`** and explain that it is the SHA-256 of the `.vdi`
   (or `.qcow2`), used to prove the machine was not modified after the defence.
2. **Change the hostname** of the machine to their login, **reboot**, and show
   that the new hostname is still there. Then restore it.
3. **Create a new user**, give it a password that respects the policy (which
   means the policy is understood: 10 characters, upper, lower, digit, no more
   than 3 identical in a row, not the login, 7 new characters), and add it to a
   new group called `evaluating`.
4. **Explain `monitoring.sh`** line by line, and interrupt it without modifying
   it (the answer is `pkill -f monitoring.sh`, or `Ctrl-C` on the `wall` call —
   never editing the file).

`docs/en.subject.md` quotes the exact wording of each requirement, and
`scripts/verify.sh` prints one line per requirement so nothing is missed.

---

## Resources

### Documentation read

- `man` pages: `sshd_config(5)`, `sudoers(5)`, `pam_pwquality(8)`,
  `pam_unix(8)`, `login.defs(5)`, `cryptsetup(8)`, `lvm(8)`, `ufw(8)`,
  `crontab(5)`, `lighttpd.conf(5)`, `fail2ban(1)`.
- Debian Administrator's Handbook — *Securing a Linux server*, *LVM*,
  *Encrypted partitions*.
- 42 subject, version 5.2, transcribed in `docs/en.subject.md`.
- libpwquality source (`pam_pwquality.c`) — to confirm that `reject_username`
  is accepted as a compatibility alias of `usercheck`.

### AI usage disclosure

In accordance with the subject's chapter on honesty, this is what AI was used
for, and what it was not used for:

- **Used for**: generating the test harness (`tests/vm.sh`,
  `tests/.vm-inside.sh`), the transcriptions of the subject, the Markdown
  documentation, and the review of the shell scripts. The emulator exists
  because the project had to be delivered and proven in an environment that
  cannot run a virtual machine.
- **Not used for**: choosing the answers to the defence questions. Every
  question in chapter *Questions* is answered from the man pages and the
  subject itself, and the reasoning is spelled out in `docs/` rather than
  pasted.

### Comparison tables (chapter VI of the subject)

| | Debian | Rocky Linux |
| --- | --- | --- |
| Origin | Community project (Debian Project), founded 1993 | Enterprise rebuild of RHEL, founded 2021 |
| Governance | Elected leader, social contract, DFSG | Company-backed (Rocky Enterprise Software Foundation) |
| Package manager | `apt` / `dpkg` (.deb) | `dnf` / `rpm` (.rpm) |
| Release cycle | ~2 years, 5 years of support | ~10 years, aligned with RHEL |
| Default MAC | AppArmor (path-based) | SELinux (label-based, enforcing) |
| Firewall | `ufw` (front-end to nftables) | `firewalld` (zones) |
| Init | systemd | systemd |
| Target use | Servers, desktops, containers | RHEL-compatible enterprise servers |

| | AppArmor | SELinux |
| --- | --- | --- |
| Model | **Path-based**: rules name files and paths | **Label-based**: every object carries a security context, policies are compiled from types |
| Where the policy lives | `/etc/apparmor.d/` | `/etc/selinux/`, `semanage`, `setsebool` |
| Learning curve | Low — one profile per program, readable syntax | High — contexts, booleans, `audit2allow` |
| Granularity | Good for confining a single daemon | Fine-grained across the whole system |
| Default on | Debian, Ubuntu, openSUSE | RHEL, Fedora, Rocky |

| | UFW | firewalld |
| --- | --- | --- |
| Front-end to | nftables (iptables on older releases) | nftables |
| Model | Flat list of rules, evaluated in order | **Zones**, each with its own default policy |
| Runtime / permanent | One configuration (`ufw reload` to apply) | Two configurations, `--permanent` plus `--reload` |
| Typical command | `ufw allow 4242/tcp` | `firewall-cmd --add-port=4242/tcp --permanent` |
| Default on | Debian | Rocky, RHEL, Fedora |

| | VirtualBox | UTM |
| --- | --- | --- |
| Hosts | x86-64 (Intel/AMD) | Apple Silicon (and Intel Macs) |
| Virtualisation | Hardware VT-x/AMD-V, own hypervisor | QEMU under the hood, native on Apple Silicon |
| Format | `.vdi` (also `.vmdk`, `.qcow2`) | `.qcow2` |
| 42 recommendation | Mandatory on Intel/AMD | Mandatory on Apple Silicon |
| Snapshots | Built-in, easy to take | Built-in |

---

## The implementation, requirement by requirement

### Chapter V — mandatory part

| Requirement (v5.2) | Where it is implemented | How it is checked |
| --- | --- | --- |
| No graphical server (else grade 0) | `scripts/setup/00-system.sh` §4 | `verify.sh`: no `xserver-xorg`/`wayland`/`gdm3` installed, default target `multi-user` |
| Hostname `<login>42` | `00-system.sh` §1 | `hostname` = `jojo835642` |
| `/etc/hosts` follows the hostname | `00-system.sh` §2 | `127.0.1.1 jojo835642` |
| Latest stable Debian | `config/vm.conf` | `docs/` |
| AppArmor running at boot | `00-system.sh` §5 | `systemctl is-enabled apparmor` |
| ≥ 2 encrypted LVM partitions | `scripts/setup/10-partition.sh` + `config/partitions.layout` | `lsblk` shows `crypt` + 7 `lvm` volumes, `findmnt /` on `/dev/mapper/` |
| SSH on 4242, root login impossible | `scripts/setup/30-ssh.sh` | `sshd -t`, `Port 4242`, `PermitRootLogin no`, listener present |
| UFW active at boot, only 4242 | `scripts/setup/40-ufw.sh` | `ufw status` active, rule for 4242, `systemctl is-enabled ufw` |
| User in `user42` **and** `sudo` | `scripts/setup/20-users.sh` | `id -nG jojo8356` |
| Password expires every 30 days | `50-password.sh` (`login.defs`) | `PASS_MAX_DAYS 30`, `chage -l` |
| Minimum 2 days between changes | idem | `PASS_MIN_DAYS 2` |
| Warning 7 days before expiry | idem | `PASS_WARN_AGE 7` |
| ≥ 10 characters, upper, lower, digit | `50-password.sh` (`pam_pwquality`) | `minlen=10 ucredit=-1 lcredit=-1 dcredit=-1` |
| ≤ 3 consecutive identical characters | idem | `maxrepeat=3` |
| No username in the password | idem | `usercheck=1` + `reject_username` |
| 7 characters not in the old password | idem | `difok=7` |
| Root password follows the policy | idem | `enforce_for_root` |
| **All** passwords changed | `20-users.sh` / `50-password.sh` | `chage -l` on root and the user |
| sudo: 3 attempts | `60-sudo.sh` | `passwd_tries=3` in `/etc/sudoers` |
| sudo: custom bad-password message | `60-sudo.sh` | `badpass_message="Nice try. …"` |
| sudo: every action logged, input **and** output | `60-sudo.sh` | `log_input`, `log_output`, `iolog_dir=/var/log/sudo` |
| sudo: log in `/var/log/sudo/` | `60-sudo.sh` | directory exists, `logfile=` |
| sudo: TTY mode | `60-sudo.sh` | `requiretty` |
| sudo: restricted paths | `60-sudo.sh` | `secure_path` (and the stock Debian line is removed so there is exactly one) |
| `monitoring.sh` with 12 fields | `scripts/monitoring.sh` | `verify.sh` checks each of the twelve labels |
| Shown by `wall` at boot and every 10 min | `scripts/setup/70-monitoring.sh` | `born2beroot-monitoring.service` + `*/10` crontab |
| No error in the output | `monitoring.sh` (every command has `2>/dev/null`) | `verify.sh` greps the output for `error` |
| README + `signature.txt` | this file, `scripts/signature.sh` | `tests/cases/00-static.sh` |

> **`signature.txt` is generated, not committed.** It is the SHA-256 of the
> virtual disk, so it only exists once a real machine has been built:
> `scripts/signature.sh <disk>.vdi` writes it, `--check` verifies it. Committing
> a hash of a disk that lives on someone else's computer would be meaningless,
> so `.gitignore` excludes it and the file is produced on the machine that is
> actually defended.

### Chapter VII — bonus part

| Requirement | Implementation |
| --- | --- |
| Correct partitioning, ≥ 7 partitions | `config/partitions.layout`: `/boot` + `root`, `swap`, `home`, `var`, `srv`, `tmp`, `var--log` |
| WordPress with lighttpd, MariaDB and PHP | `scripts/setup/80-bonus-wordpress.sh` |
| MariaDB reachable only from the machine | `bind-address = 127.0.0.1`, UFW rule restricted to `lo` |
| lighttpd serves WordPress on port 80 | `server.document-root = /var/www/html`, FastCGI → `php-cgi` |
| One extra service, not NGINX or Apache | **vsftpd** (FTP over TLS, port 21 + passive range 40000-40010) |
| Justified | See below |
| Extra ports open in the firewall | 80/tcp, 21/tcp and the passive range in `config/ufw_rules.sh` |
| `fail2ban` watching the exposed services | `90-bonus-service.sh`: jails `sshd` (4242) and `vsftpd` (21) |

#### Why vsftpd

The subject forbids NGINX and Apache2 as the extra service, and asks for
something *useful*. **vsftpd** was chosen because:

- it is a genuinely different piece of infrastructure, not a second web server;
- it can be confined and encrypted (FTPS with a self-signed certificate, TLS
  required for both control and data channels);
- each user is chrooted in its own directory (`chroot_local_user=YES`,
  `allow_writeable_chroot=YES`), so a compromised account cannot walk the
  filesystem;
- it is a realistic way to move a WordPress archive or a database dump onto a
  server that only exposes 4242 otherwise;
- `fail2ban` then has a second, independent service to protect (`vsftpd` jail),
  which demonstrates that the firewall and the intrusion detection are not just
  copied from the mandatory part.

Four extra ports are opened as a consequence: `80/tcp` (lighttpd), `21/tcp`
(vsftpd control), `40000-40010/tcp` (vsftpd passive data) and `3306/tcp` bound
to `lo` only (MariaDB, never reachable from outside). `config/ufw_rules.sh`
contains the exact commands and the comment explaining each one.

---

## Testing

```bash
bash tests/run_tests.sh            # static checks + 8 requirement cases + community testers
bash tests/run_tests.sh --static   # the fast half (no boot)
bash tests/run_tests.sh --bonus    # test the machine with the bonus part installed
bash tests/run_community.sh        # only the five third-party testers
```

`tests/run_tests.sh` prints a summary per case and exits non-zero if any check
fails. The community testers are kept **verbatim** in `tests/community/` (only
their file names were normalised) and are run against the emulator, which is
the strongest evidence that the machine really behaves like the one they were
written for:

| Tester | Checks | Result |
| --- | --- | --- |
| gemartin99/Born2beroot-Tester | 23 | all pass |
| highrulez/Born2BeRoot-Tester | 28 | all pass |
| pandashaly/Born2BeRoot_tester | 8 | all pass |
| Othello42/Born2BeRoot-tester | 13 | all pass |
| Pixailz/Born2BeRootTester | 8 | all pass |

---

## What the emulator does and does not do

**Does**: run every project script unmodified against a real filesystem, with a
real dpkg database, a real PAM configuration, a real `visudo`, a real crontab
spool and a real lighttpd/PHP endpoint; enforce the same file modes and
ownership; survive a reboot (the image is synced back after every command).

**Does not**: encrypt a real disk (the LUKS2 container and the LVM volumes are
declared in `config/partitions.layout` and simulated by `lsblk`/`vgs`/`lvs`),
boot a kernel, or run systemd. On a real machine, `scripts/setup.sh` uses the
real tools; the emulator is a faithful *stand-in*, not a replacement.

---

## Licence

Code under the repository's top-level licence. The official subject PDF
(`docs/en.subject.pdf`) is the property of 42 and is included for reference
only. The community testers under `tests/community/` keep their original
licences, quoted at the top of each file.
