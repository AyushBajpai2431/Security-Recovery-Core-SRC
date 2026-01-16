# Security Recovery Core - Security Model

## Threat Model

### Protected Against

1. **Firmware Corruption**
   - Accidental corruption during updates
   - Malware-induced firmware modification
   - Hardware failures during flash operations

2. **Permanent Bricking**
   - Failed firmware updates
   - Corrupted bootloader
   - Invalid firmware images

3. **Unauthorized Disable**
   - Malware attempting to disable recovery
   - Accidental disable without authentication
   - OS-level tampering

### Not Protected Against

1. **Physical Access Attacks**
   - Direct SPI flash manipulation with hardware tools
   - EC/MCU replacement
   - Hardware-level attacks

2. **Supply Chain Attacks**
   - Compromised firmware at source
   - Malicious hardware components
   - Pre-installed backdoors

3. **Advanced Persistent Threats (APTs)**
   - Nation-state level attacks
   - Sophisticated hardware implants
   - Zero-day exploits in firmware

## Security Architecture

### Cryptographic Protection

#### Signature Verification

- **Algorithm:** ECDSA-P256 or RSA-2048
- **Hash Function:** SHA-256
- **Key Storage:** Hardware-protected (TPM, secure element, or locked SPI region)

#### Key Management

- **Public Key:** Embedded in Recovery Core firmware
- **Private Key:** Stored securely (hardware security module recommended)
- **Key Rotation:** Supported via firmware update

### Tamper Resistance

#### Logging System

- **Storage:** Tamper-resistant region in SPI flash
- **Integrity:** Cryptographic hashing of log entries
- **Access Control:** Read-only via authenticated interface

#### Configuration Protection

- **Storage:** Reserved SPI flash region
- **Integrity Checks:** CRC32 + cryptographic signature
- **Write Protection:** Hardware lock when possible

### Boot Detection Security

#### Multiple Redundant Methods

1. **GPIO Signal**
   - Hardware-level success indicator
   - Cannot be spoofed by software alone

2. **Watchdog Timer**
   - Hardware timer cleared by successful boot
   - Independent of OS/kernel

3. **POST Codes**
   - BIOS/UEFI progress codes
   - Threshold-based detection

4. **Firmware Flag**
   - Set by firmware after successful initialization
   - Stored in NVRAM/SPI

#### Timeout Protection

- **Boot Timeout:** 30 seconds (configurable)
- **Failure Detection:** If no success signal within timeout
- **Recovery Trigger:** Automatic USB recovery attempt

## Anti-Abuse Mechanisms

### Unauthorized Disable Prevention

1. **Password Protection**
   - OS-level authentication required
   - Prevents script-based disable

2. **Time-Limited Disable**
   - Maximum 7 days disable duration
   - Automatic re-enable after expiration
   - Warning messages to user

3. **Logging**
   - All disable/enable operations logged
   - Timestamp and user information recorded

### Malware Protection

1. **Hardware-Level Operation**
   - Recovery Core runs before OS
   - Cannot be disabled by malware running in OS

2. **Signature Verification**
   - All firmware images must be signed
   - Prevents downgrade attacks
   - Blocks malicious firmware

3. **Immutable Core**
   - Recovery Core firmware is write-protected
   - Cannot be modified without physical access

### Uninstall Protection

1. **Multi-Step Confirmation**
   - 4 separate confirmation questions
   - OS password required
   - 5-second final warning

2. **Validation Checks**
   - Firmware integrity verified before removal
   - System state validated
   - Abort on any failure

3. **Safe Removal Process**
   - Removal scheduled, not immediate
   - Completes on reboot (controlled environment)
   - Rollback possible if interrupted

## Security Best Practices

### For End Users

1. **Keep Backups Updated**
   - Ensure USB recovery device is present
   - Verify backups are recent (< 10 minutes old)

2. **Protect USB Recovery Device**
   - Store in secure location
   - Encrypt if containing sensitive firmware
   - Keep separate from system

3. **Monitor Status**
   - Regularly check `security status`
   - Review logs for anomalies
   - Report suspicious activity

4. **Use Strong Authentication**
   - Strong OS passwords
   - Enable full disk encryption
   - Use secure boot (where supported)

### For Administrators

1. **Key Management**
   - Store private keys in HSM
   - Implement key rotation policy
   - Secure key backup procedures

2. **Access Control**
   - Limit who can disable recovery
   - Monitor disable/enable operations
   - Audit logs regularly

3. **Firmware Updates**
   - Always sign firmware updates
   - Test updates in non-production
   - Maintain rollback capability

4. **Incident Response**
   - Document recovery procedures
   - Maintain firmware backups
   - Test recovery process regularly

## Known Limitations

### Platform Dependencies

- **SPI Flash Access:** Requires appropriate drivers/permissions
- **USB Support:** Platform-specific USB stack required
- **Boot Detection:** Hardware-dependent methods

### Recovery Limitations

- **USB Dependency:** Recovery requires USB device present
- **Signature Verification:** Requires valid cryptographic keys
- **Firmware Compatibility:** Only compatible firmware can be restored

### Security Limitations

- **Physical Access:** Cannot prevent hardware-level attacks
- **Supply Chain:** Cannot detect pre-installed compromises
- **Zero-Day Exploits:** May be vulnerable to unknown firmware flaws

## Security Updates

### Vulnerability Reporting

Report security issues to: security@src-project.org

### Update Process

1. Security patches released promptly
2. Firmware updates signed with new keys if needed
3. Users notified via status command
4. Automatic update recommendation

## Compliance

### Standards Alignment

- **NIST SP 800-147:** BIOS Protection Guidelines
- **NIST SP 800-193:** Platform Firmware Resiliency
- **Common Criteria:** Protection Profile compatible

### Certifications

- Work towards FIPS 140-2 compliance (crypto modules)
- Hardware security module integration
- Platform-specific certifications as available

## Security Audit

### Regular Audits

- Code review for security issues
- Penetration testing
- Cryptographic review
- Hardware security assessment

### Audit Results

- Published security advisories
- CVE assignments for vulnerabilities
- Patch release schedule

## References

- [NIST SP 800-147](https://csrc.nist.gov/publications/detail/sp/800-147/final)
- [NIST SP 800-193](https://csrc.nist.gov/publications/detail/sp/800-193/final)
- [UEFI Secure Boot](https://uefi.org/specifications)
- [TPM Specifications](https://trustedcomputinggroup.org/work-groups/trusted-platform-module/)
