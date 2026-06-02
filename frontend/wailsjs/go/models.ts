export namespace profile {
	
	export class ScanProfile {
	    name: string;
	    discovery_mode: string;
	    timeout_ms: number;
	    concurrency: number;
	    resolve_dns: boolean;
	    resolve_mac: boolean;
	    export_formats: string[];
	    ports?: number[];
	
	    static createFrom(source: any = {}) {
	        return new ScanProfile(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.name = source["name"];
	        this.discovery_mode = source["discovery_mode"];
	        this.timeout_ms = source["timeout_ms"];
	        this.concurrency = source["concurrency"];
	        this.resolve_dns = source["resolve_dns"];
	        this.resolve_mac = source["resolve_mac"];
	        this.export_formats = source["export_formats"];
	        this.ports = source["ports"];
	    }
	}

}

export namespace results {
	
	export class HostResult {
	    ip: string;
	    hostname?: string;
	    mac?: string;
	    vendor?: string;
	    rtt_ms?: number;
	    alive: boolean;
	    source?: string;
	    last_seen_utc?: string;
	    open_ports?: number[];
	
	    static createFrom(source: any = {}) {
	        return new HostResult(source);
	    }
	
	    constructor(source: any = {}) {
	        if ('string' === typeof source) source = JSON.parse(source);
	        this.ip = source["ip"];
	        this.hostname = source["hostname"];
	        this.mac = source["mac"];
	        this.vendor = source["vendor"];
	        this.rtt_ms = source["rtt_ms"];
	        this.alive = source["alive"];
	        this.source = source["source"];
	        this.last_seen_utc = source["last_seen_utc"];
	        this.open_ports = source["open_ports"];
	    }
	}

}

