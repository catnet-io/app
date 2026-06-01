export namespace scanner {
	
	export class DeviceInfo {
	    ip: string;
	    isAlive: boolean;
	    hostname: string;
	    mac: string;
	    openPortsCount: number;
	    openPorts: number[];
	
	    static createFrom(source: any = {}) {
	        return new DeviceInfo(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.ip = source["ip"];
	        this.isAlive = source["isAlive"];
	        this.hostname = source["hostname"];
	        this.mac = source["mac"];
	        this.openPortsCount = source["openPortsCount"];
	        this.openPorts = source["openPorts"];
	    }
	}
	export class ScanConfig {
	    defaultPorts: number[];
	    portTimeoutMs: number;
	    pingTimeoutMs: number;
	    maxThreads: number;
	
	    static createFrom(source: any = {}) {
	        return new ScanConfig(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.defaultPorts = source["defaultPorts"];
	        this.portTimeoutMs = source["portTimeoutMs"];
	        this.pingTimeoutMs = source["pingTimeoutMs"];
	        this.maxThreads = source["maxThreads"];
	    }
	}

}

